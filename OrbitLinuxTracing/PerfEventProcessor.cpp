// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventProcessor.h"

#include <OrbitBase/Logging.h>

#include <memory>
#include <queue>

#include "LinuxTracingUtils.h"
#include "PerfEvent.h"

namespace LinuxTracing {

void PerfEventQueue::PushEvent(int origin_fd, std::unique_ptr<PerfEvent> event) {
  if (fd_event_queues_.count(origin_fd) > 0) {
    std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>> event_queue =
        fd_event_queues_.at(origin_fd);
    CHECK(!event_queue->empty());
    // Fundamental assumption: events from the same file descriptor come already
    // in order.
    CHECK(event->GetTimestamp() >= event_queue->front()->GetTimestamp());
    event_queue->push(std::move(event));
  } else {
    auto event_queue = std::make_shared<std::queue<std::unique_ptr<PerfEvent>>>();
    fd_event_queues_.insert(std::make_pair(origin_fd, event_queue));
    event_queue->push(std::move(event));
    event_queues_queue_.push(std::make_pair(origin_fd, event_queue));
  }
}

bool PerfEventQueue::HasEvent() { return !event_queues_queue_.empty(); }

PerfEvent* PerfEventQueue::TopEvent() { return event_queues_queue_.top().second->front().get(); }

std::unique_ptr<PerfEvent> PerfEventQueue::PopEvent() {
  std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>> top_fd_queue =
      event_queues_queue_.top();
  event_queues_queue_.pop();
  const int& top_fd = top_fd_queue.first;
  std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>& top_queue = top_fd_queue.second;

  std::unique_ptr<PerfEvent> top_event = std::move(top_queue->front());
  top_queue->pop();
  if (top_queue->empty()) {
    fd_event_queues_.erase(top_fd);
  } else {
    // Remove and re-insert so that the queue is in the right position in the
    // heap after the front of the queue has been removed.
    event_queues_queue_.push(top_fd_queue);
  }

  return top_event;
}

void PerfEventProcessor::AddEvent(int origin_fd, std::unique_ptr<PerfEvent> event) {
  if (last_processed_timestamp_ns_ > 0 && event->GetTimestamp() < last_processed_timestamp_ns_) {
    if (discarded_out_of_order_counter_ != nullptr) {
      ++(*discarded_out_of_order_counter_);
    }
    return;
  }
  event_queue_.PushEvent(origin_fd, std::move(event));
}

void PerfEventProcessor::ProcessAllEvents() {
  CHECK(!visitors_.empty());
  while (event_queue_.HasEvent()) {
    std::unique_ptr<PerfEvent> event = event_queue_.PopEvent();
    // Events are guaranteed to be processed in order of timestamp
    // as out-of-order events are discarded in AddEvent.
    CHECK(event->GetTimestamp() >= last_processed_timestamp_ns_);
    last_processed_timestamp_ns_ = event->GetTimestamp();
    for (PerfEventVisitor* visitor : visitors_) {
      event->Accept(visitor);
    }
  }
}

void PerfEventProcessor::ProcessOldEvents() {
  CHECK(!visitors_.empty());
  uint64_t current_timestamp_ns = MonotonicTimestampNs();

  while (event_queue_.HasEvent()) {
    PerfEvent* event = event_queue_.TopEvent();

    // Do not read the most recent events as out-of-order events could (and will) arrive.
    if (event->GetTimestamp() + kProcessingDelayMs * 1'000'000 >= current_timestamp_ns) {
      break;
    }
    // Events are guaranteed to be processed in order of timestamp
    // as out-of-order events are discarded in AddEvent.
    CHECK(event->GetTimestamp() >= last_processed_timestamp_ns_);
    last_processed_timestamp_ns_ = event->GetTimestamp();

    for (PerfEventVisitor* visitor : visitors_) {
      event->Accept(visitor);
    }
    event_queue_.PopEvent();
  }
}

}  // namespace LinuxTracing
