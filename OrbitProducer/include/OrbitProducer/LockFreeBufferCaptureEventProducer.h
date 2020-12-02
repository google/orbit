// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_
#define ORBIT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_

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
// The translation from IntermediateEventT to CaptureEvent is handled by
// TranslateIntermediateEvent, which subclasses need to implement.
template <typename IntermediateEventT>
class LockFreeBufferCaptureEventProducer : public CaptureEventProducer {
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
  void OnCaptureStart() override {
    absl::MutexLock lock{&should_send_all_events_sent_mutex_};
    should_send_all_events_sent_ = false;
  }

  void OnCaptureStop() override {
    absl::MutexLock lock{&should_send_all_events_sent_mutex_};
    should_send_all_events_sent_ = true;
  }

  // Subclasses need to implement this method to convert an IntermediateEventT enqueued
  // in the internal lock-free buffer to a CaptureEvent to be sent to ProducerSideService.
  [[nodiscard]] virtual orbit_grpc_protos::CaptureEvent TranslateIntermediateEvent(
      IntermediateEventT&& intermediate_event) = 0;

 private:
  void ForwarderThread() {
    constexpr uint64_t kMaxEventsPerRequest = 10'000;
    std::vector<IntermediateEventT> dequeued_events(kMaxEventsPerRequest);
    while (!shutdown_requested_) {
      size_t dequeued_event_count;
      while ((dequeued_event_count = lock_free_queue_.try_dequeue_bulk(dequeued_events.begin(),
                                                                       kMaxEventsPerRequest)) > 0) {
        orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest send_request;
        auto* capture_events =
            send_request.mutable_buffered_capture_events()->mutable_capture_events();
        for (size_t i = 0; i < dequeued_event_count; ++i) {
          orbit_grpc_protos::CaptureEvent* event = capture_events->Add();
          *event = TranslateIntermediateEvent(std::move(dequeued_events[i]));
        }

        if (!SendCaptureEvents(send_request)) {
          ERROR("Forwarding %lu CaptureEvents", dequeued_event_count);
          break;
        }
        if (dequeued_event_count < kMaxEventsPerRequest) {
          break;
        }
      }

      // lock_free_queue_ is now empty: check if we need to send AllEventsSent.
      {
        should_send_all_events_sent_mutex_.Lock();
        if (should_send_all_events_sent_) {
          should_send_all_events_sent_ = false;
          should_send_all_events_sent_mutex_.Unlock();
          if (!NotifyAllEventsSent()) {
            ERROR("Notifying that all CaptureEvents have been sent");
          }
          continue;
        }
        should_send_all_events_sent_mutex_.Unlock();
      }

      static constexpr std::chrono::duration kSleepOnEmptyQueue = std::chrono::microseconds{100};
      // Wait for lock_free_queue_ to fill up with new CaptureEvents.
      std::this_thread::sleep_for(kSleepOnEmptyQueue);
    }
  }

 private:
  moodycamel::ConcurrentQueue<IntermediateEventT> lock_free_queue_;

  std::thread forwarder_thread_;
  std::atomic<bool> shutdown_requested_ = false;

  bool should_send_all_events_sent_ = false;
  absl::Mutex should_send_all_events_sent_mutex_;
};

}  // namespace orbit_producer

#endif  // ORBIT_PRODUCER_LOCK_FREE_BUFFER_CAPTURE_EVENT_PRODUCER_H_
