// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_EVENT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_
#define CAPTURE_EVENT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_

#include <google/protobuf/arena.h>

#include "CaptureEventProducer/CaptureEventProducer.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/ThreadUtils.h"
#include "concurrentqueue.h"

namespace orbit_capture_event_producer {

// This still abstract implementation of CaptureEventProducer provides a lock-free queue where to
// write events with low overhead from the fast path where they are produced.
// Events are enqueued using the methods EnqueueIntermediateEvent(IfCapturing).
//
// Internally, a thread reads from the lock-free queue and sends ProducerCaptureEvents to
// ProducerSideService using the methods provided by the superclass.
//
// The type of the events stored in the lock-free queue is specified by the type parameter
// IntermediateEventT. These events don't need to be ProducerCaptureEvents, nor protobufs at all.
// This is to allow enqueuing objects that are faster to produce than protobufs.
// ProducerCaptureEvents are then built from IntermediateEventT in TranslateIntermediateEvent, which
// subclasses need to implement.
//
// In particular, when hundreds of thousands of events are produced per second, it is recommended
// that IntermediateEventT not be a protobuf or another type that involves heap allocations, as the
// cost of dynamic allocations and de-allocations can add up quickly.
template <typename IntermediateEventT>
class LockFreeBufferCaptureEventProducer : public CaptureEventProducer {
 public:
  void BuildAndStart(const std::shared_ptr<grpc::Channel>& channel) final {
    CaptureEventProducer::BuildAndStart(channel);

    forwarder_thread_ = std::thread{&LockFreeBufferCaptureEventProducer::ForwarderThread, this};
  }

  void ShutdownAndWait() final {
    shutdown_requested_ = true;

    ORBIT_CHECK(forwarder_thread_.joinable());
    forwarder_thread_.join();

    CaptureEventProducer::ShutdownAndWait();
  }

  void EnqueueIntermediateEvent(const IntermediateEventT& event) {
    lock_free_queue_.enqueue(event);
  }

  void EnqueueIntermediateEvent(IntermediateEventT&& event) {
    lock_free_queue_.enqueue(std::move(event));
  }

  bool EnqueueIntermediateEventIfCapturing(
      const std::function<IntermediateEventT()>& event_builder_if_capturing) {
    if (IsCapturing()) {
      lock_free_queue_.enqueue(event_builder_if_capturing());
      return true;
    }
    return false;
  }

 protected:
  void OnCaptureStart(orbit_grpc_protos::CaptureOptions /*capture_options*/) override {
    absl::MutexLock lock{&status_mutex_};
    status_ = ProducerStatus::kShouldSendEvents;
  }

  void OnCaptureStop() override {
    absl::MutexLock lock{&status_mutex_};
    status_ = ProducerStatus::kShouldNotifyAllEventsSent;
  }

  void OnCaptureFinished() override {
    absl::MutexLock lock{&status_mutex_};
    status_ = ProducerStatus::kShouldDropEvents;
  }

  // Subclasses need to implement this method to convert an `IntermediateEventT` enqueued in the
  // internal lock-free buffer to a `CaptureEvent` to be sent to ProducerSideService.
  // The `CaptureEvent` must be created in the Arena using `google::protobuf::Arena::CreateMessage`
  // from <google/protobuf/arena.h>. The pointer provided by `CreateMessage` should be returned.
  // This optimizes memory allocations and cache efficiency. But keep in mind that:
  // - `string` and `bytes` fields (both of which use `std::string` internally) still get heap
  //   allocated no matter what;
  // - If `IntermediateEventT` is itself a `ProducerCaptureEvent`, or the type of one of its fields,
  //   attempting to move from it into the Arena-allocated `ProducerCaptureEvent` will silently
  //   result in a deep copy.
  [[nodiscard]] virtual orbit_grpc_protos::ProducerCaptureEvent* TranslateIntermediateEvent(
      IntermediateEventT&& intermediate_event, google::protobuf::Arena* arena) = 0;

 private:
  void ForwarderThread() {
    orbit_base::SetCurrentThreadName("ForwarderThread");

    constexpr uint64_t kMaxEventsPerRequest = 10'000;
    std::vector<IntermediateEventT> dequeued_events(kMaxEventsPerRequest);

    // Pre-allocate and always reuse the same 1 MB chunk of memory as the first block of each Arena
    // instance in the loop below. This is a small but measurable performance improvement.
    google::protobuf::ArenaOptions arena_options;
    constexpr size_t kArenaFixedBlockSize = 1024 * 1024;
    auto arena_initial_block = make_unique_for_overwrite<char[]>(kArenaFixedBlockSize);
    arena_options.initial_block = arena_initial_block.get();
    arena_options.initial_block_size = kArenaFixedBlockSize;
    // Also make sure that, if the Arena still needs to allocate more blocks, those are larger than
    // the default, which would be capped at 8 kB. We choose the same size as the pre-allocated
    // first block for simplicity.
    arena_options.start_block_size = kArenaFixedBlockSize;
    arena_options.max_block_size = kArenaFixedBlockSize;

    while (!shutdown_requested_) {
      while (true) {
        size_t dequeued_event_count =
            lock_free_queue_.try_dequeue_bulk(dequeued_events.begin(), kMaxEventsPerRequest);
        bool queue_was_emptied = dequeued_event_count < kMaxEventsPerRequest;

        ProducerStatus current_status;
        {
          absl::MutexLock lock{&status_mutex_};
          current_status = status_;
          if (status_ == ProducerStatus::kShouldNotifyAllEventsSent && queue_was_emptied) {
            // We are about to send AllEventsSent: update status_ while we hold the mutex.
            status_ = ProducerStatus::kShouldDropEvents;
          }
        }

        if ((current_status == ProducerStatus::kShouldSendEvents ||
             current_status == ProducerStatus::kShouldNotifyAllEventsSent) &&
            dequeued_event_count > 0) {
          google::protobuf::Arena arena{arena_options};
          auto* send_request = google::protobuf::Arena::CreateMessage<
              orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>(&arena);
          auto* capture_events =
              send_request->mutable_buffered_capture_events()->mutable_capture_events();
          capture_events->Reserve(dequeued_event_count);

          for (size_t i = 0; i < dequeued_event_count; ++i) {
            capture_events->AddAllocated(
                TranslateIntermediateEvent(std::move(dequeued_events[i]), &arena));
          }

          if (!SendCaptureEvents(*send_request)) {
            ORBIT_ERROR("Forwarding %lu CaptureEvents", dequeued_event_count);
            break;
          }
        }

        if (current_status == ProducerStatus::kShouldNotifyAllEventsSent && queue_was_emptied) {
          // lock_free_queue_ is now empty and status_ == kShouldNotifyAllEventsSent,
          // send AllEventsSent. status_ has already been changed to kShouldDropEvents.
          if (!NotifyAllEventsSent()) {
            ORBIT_ERROR("Notifying that all CaptureEvents have been sent");
          }
          break;
        }

        // Note that if current_status == ProducerStatus::kShouldDropEvents
        // the events extracted from the lock_free_queue_ will just be dropped.

        if (queue_was_emptied) {
          break;
        }
      }

      constexpr std::chrono::microseconds kSleepOnEmptyQueue{1000};
      // Wait for lock_free_queue_ to fill up with new CaptureEvents.
      std::this_thread::sleep_for(kSleepOnEmptyQueue);
    }
  }

  moodycamel::ConcurrentQueue<IntermediateEventT> lock_free_queue_;

  std::thread forwarder_thread_;
  std::atomic<bool> shutdown_requested_ = false;

  enum class ProducerStatus { kShouldSendEvents, kShouldNotifyAllEventsSent, kShouldDropEvents };
  ProducerStatus status_ = ProducerStatus::kShouldDropEvents;
  absl::Mutex status_mutex_;
};

}  // namespace orbit_capture_event_producer

#endif  // CAPTURE_EVENT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_
