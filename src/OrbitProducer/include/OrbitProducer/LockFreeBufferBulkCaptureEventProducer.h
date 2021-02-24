// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_LOCK_FREE_BUFFER_BULK_CAPTURE_EVENT_PRODUCER_H_
#define ORBIT_PRODUCER_LOCK_FREE_BUFFER_BULK_CAPTURE_EVENT_PRODUCER_H_

#include <chrono>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitProducer/CaptureEventProducer.h"
#include "concurrentqueue.h"

namespace orbit_producer {

// This still abstract implementation of CaptureEventProducer provides a lock-free queue
// where to write events with low overhead from the fast path where they are produced.
// The methods EnqueueIntermediateEvent(IfCapturing) allow to enqueue those events.
//
// Internally, a thread reads from the lock-free queue and sends CaptureEvents
// to ProducerSideService using the methods provided by the superclass.
//
// Note that the events stored in the lock-free queue, whose type is specified by the
// type parameter IntermediateEventT, don't need to be CaptureEvents, nor protobufs at all.
// This is to allow enqueuing objects that are faster to produce than protobufs.
// The translation from IntermediateEventT to CaptureEvent is done in bulk by
// TranslateIntermediateEvents, which subclasses need to implement.
template <typename IntermediateEventT>
class LockFreeBufferBulkCaptureEventProducer : public CaptureEventProducer {
 public:
  void BuildAndStart(const std::shared_ptr<grpc::Channel>& channel) override {
    CaptureEventProducer::BuildAndStart(channel);

    forwarder_thread_ = std::thread{[this] { ForwarderThread(); }};
  }

  void ShutdownAndWait() override {
    shutdown_requested_ = true;

    CHECK(forwarder_thread_.joinable());
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

  // Subclasses need to implement this method to convert an IntermediateEventT enqueued
  // in the internal lock-free buffer to a CaptureEvent to be sent to ProducerSideService.
  [[nodiscard]] virtual orbit_grpc_protos::ProducerCaptureEvent TranslateIntermediateEvents(
      IntermediateEventT* intermediate_events, size_t size) = 0;

 private:
  void ForwarderThread() {
    constexpr uint64_t kMaxEventsPerRequest = 10'000;
    std::vector<IntermediateEventT> dequeued_events(kMaxEventsPerRequest);
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
          orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest send_request;
          auto* capture_events =
              send_request.mutable_buffered_capture_events()->mutable_capture_events();

          orbit_grpc_protos::ProducerCaptureEvent* event = capture_events->Add();
          *event = TranslateIntermediateEvents(dequeued_events.data(), dequeued_event_count);

          if (!SendCaptureEvents(send_request)) {
            ERROR("Forwarding %lu CaptureEvents", dequeued_event_count);
            break;
          }
        }

        if (current_status == ProducerStatus::kShouldNotifyAllEventsSent && queue_was_emptied) {
          // lock_free_queue_ is now empty and status_ == kShouldNotifyAllEventsSent,
          // send AllEventsSent. status_ has already been changed to kShouldDropEvents.
          if (!NotifyAllEventsSent()) {
            ERROR("Notifying that all CaptureEvents have been sent");
          }
          break;
        }

        // Note that if current_status == ProducerStatus::kShouldDropEvents
        // the events extracted from the lock_free_queue_ will just be dropped.

        if (queue_was_emptied) {
          break;
        }
      }

      static constexpr std::chrono::duration kSleepOnEmptyQueue = std::chrono::milliseconds{10};
      // Wait for lock_free_queue_ to fill up with new CaptureEvents.
      std::this_thread::sleep_for(kSleepOnEmptyQueue);
    }
  }

 private:
  moodycamel::ConcurrentQueue<IntermediateEventT> lock_free_queue_;

  std::thread forwarder_thread_;
  std::atomic<bool> shutdown_requested_ = false;

  enum class ProducerStatus { kShouldSendEvents, kShouldNotifyAllEventsSent, kShouldDropEvents };
  ProducerStatus status_ = ProducerStatus::kShouldDropEvents;
  absl::Mutex status_mutex_;
};

}  // namespace orbit_producer

#endif  // ORBIT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_
