// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventQueue.h"

#include <queue>

namespace LinuxTracing {

void PerfEventQueue::PushEvent(std::unique_ptr<PerfEvent> event) {
  int origin_fd = event->GetOriginFileDescriptor();

  if (auto queue_it = queues_.find(origin_fd); queue_it != queues_.end()) {
    const std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>& queue = queue_it->second;

    CHECK(!queue->empty());
    // Fundamental assumption: events from the same file descriptor come already in order.
    CHECK(event->GetTimestamp() >= queue->back()->GetTimestamp());
    queue->push(std::move(event));

  } else {
    queue_it =
        queues_.emplace(origin_fd, std::make_unique<std::queue<std::unique_ptr<PerfEvent>>>())
            .first;
    const std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>& queue = queue_it->second;

    queue->push(std::move(event));
    queues_heap_.push(queue.get());
  }
}

bool PerfEventQueue::HasEvent() { return !queues_heap_.empty(); }

PerfEvent* PerfEventQueue::TopEvent() { return queues_heap_.top()->front().get(); }

std::unique_ptr<PerfEvent> PerfEventQueue::PopEvent() {
  std::queue<std::unique_ptr<PerfEvent>>* top_queue = queues_heap_.top();
  // Always remove the queue with the older PerfEvent. If it's not empty, it will be re-inserted,
  // so that it's moved to the correct position. See below.
  queues_heap_.pop();

  std::unique_ptr<PerfEvent> top_event = std::move(top_queue->front());
  top_queue->pop();
  if (top_queue->empty()) {
    int top_fd = top_event->GetOriginFileDescriptor();
    queues_.erase(top_fd);
  } else {
    // The queue with the older PerfEvent is always removed. If it's not empty, re-insert it so that
    // it is in the correct position in the heap after the front of the queue has been removed.
    queues_heap_.push(top_queue);
  }

  return top_event;
}

}  // namespace LinuxTracing
