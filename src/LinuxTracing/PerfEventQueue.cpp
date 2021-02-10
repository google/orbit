// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventQueue.h"

#include <absl/meta/type_traits.h>
#include <stddef.h>

#include <algorithm>
#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

void PerfEventQueue::PushEvent(std::unique_ptr<PerfEvent> event) {
  int origin_fd = event->GetOrderedInFileDescriptor();
  if (origin_fd == PerfEvent::kNotOrderedInAnyFileDescriptor) {
    priority_queue_of_events_not_ordered_by_fd_.push(std::move(event));

  } else if (auto queue_it = queues_of_events_ordered_by_fd_.find(origin_fd);
             queue_it != queues_of_events_ordered_by_fd_.end()) {
    const std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>& queue = queue_it->second;

    CHECK(!queue->empty());
    // Fundamental assumption: events from the same file descriptor come already in order.
    CHECK(event->GetTimestamp() >= queue->back()->GetTimestamp());
    queue->push(std::move(event));

  } else {
    queue_it = queues_of_events_ordered_by_fd_
                   .emplace(origin_fd, std::make_unique<std::queue<std::unique_ptr<PerfEvent>>>())
                   .first;
    const std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>& queue = queue_it->second;

    queue->push(std::move(event));
    heap_of_queues_of_events_ordered_by_fd_.emplace_back(queue.get());
    MoveUpBackOfHeapOfQueues();
  }
}

bool PerfEventQueue::HasEvent() const {
  return !heap_of_queues_of_events_ordered_by_fd_.empty() ||
         !priority_queue_of_events_not_ordered_by_fd_.empty();
}

PerfEvent* PerfEventQueue::TopEvent() {
  // As we effectively have two priority queues, get the older event between the two events at the
  // top of the two queues.
  PerfEvent* top_event = nullptr;
  if (!priority_queue_of_events_not_ordered_by_fd_.empty()) {
    top_event = priority_queue_of_events_not_ordered_by_fd_.top().get();
  }
  if (!heap_of_queues_of_events_ordered_by_fd_.empty() &&
      (top_event == nullptr ||
       heap_of_queues_of_events_ordered_by_fd_.front()->front()->GetTimestamp() <
           top_event->GetTimestamp())) {
    top_event = heap_of_queues_of_events_ordered_by_fd_.front()->front().get();
  }
  CHECK(top_event != nullptr);
  return top_event;
}

std::unique_ptr<PerfEvent> PerfEventQueue::PopEvent() {
  if (!priority_queue_of_events_not_ordered_by_fd_.empty() &&
      (heap_of_queues_of_events_ordered_by_fd_.empty() ||
       priority_queue_of_events_not_ordered_by_fd_.top()->GetTimestamp() <
           heap_of_queues_of_events_ordered_by_fd_.front()->front()->GetTimestamp())) {
    // The oldest event is at the top of the priority queue holding the events that cannot be
    // assumed sorted in any ring buffer.
    std::unique_ptr<PerfEvent> top_event = std::move(
        const_cast<std::unique_ptr<PerfEvent>&>(priority_queue_of_events_not_ordered_by_fd_.top()));
    priority_queue_of_events_not_ordered_by_fd_.pop();
    return top_event;
  }

  std::queue<std::unique_ptr<PerfEvent>>* top_queue =
      heap_of_queues_of_events_ordered_by_fd_.front();
  std::unique_ptr<PerfEvent> top_event = std::move(top_queue->front());
  top_queue->pop();

  if (top_queue->empty()) {
    int top_fd = top_event->GetOrderedInFileDescriptor();
    queues_of_events_ordered_by_fd_.erase(top_fd);
    std::swap(heap_of_queues_of_events_ordered_by_fd_.front(),
              heap_of_queues_of_events_ordered_by_fd_.back());
    heap_of_queues_of_events_ordered_by_fd_.pop_back();
    MoveDownFrontOfHeapOfQueues();
  } else {
    MoveDownFrontOfHeapOfQueues();
  }

  return top_event;
}

void PerfEventQueue::MoveDownFrontOfHeapOfQueues() {
  if (heap_of_queues_of_events_ordered_by_fd_.empty()) {
    return;
  }

  size_t current_index = 0;
  size_t new_index;
  while (true) {
    new_index = current_index;
    size_t left_index = current_index * 2 + 1;
    size_t right_index = current_index * 2 + 2;
    if (left_index < heap_of_queues_of_events_ordered_by_fd_.size() &&
        heap_of_queues_of_events_ordered_by_fd_[left_index]->front()->GetTimestamp() <
            heap_of_queues_of_events_ordered_by_fd_[new_index]->front()->GetTimestamp()) {
      new_index = left_index;
    }
    if (right_index < heap_of_queues_of_events_ordered_by_fd_.size() &&
        heap_of_queues_of_events_ordered_by_fd_[right_index]->front()->GetTimestamp() <
            heap_of_queues_of_events_ordered_by_fd_[new_index]->front()->GetTimestamp()) {
      new_index = right_index;
    }
    if (new_index != current_index) {
      std::swap(heap_of_queues_of_events_ordered_by_fd_[new_index],
                heap_of_queues_of_events_ordered_by_fd_[current_index]);
      current_index = new_index;
    } else {
      break;
    }
  }
}

void PerfEventQueue::MoveUpBackOfHeapOfQueues() {
  if (heap_of_queues_of_events_ordered_by_fd_.empty()) {
    return;
  }

  size_t current_index = heap_of_queues_of_events_ordered_by_fd_.size() - 1;
  while (current_index > 0) {
    size_t parent_index = (current_index - 1) / 2;
    if (heap_of_queues_of_events_ordered_by_fd_[parent_index]->front()->GetTimestamp() <=
        heap_of_queues_of_events_ordered_by_fd_[current_index]->front()->GetTimestamp()) {
      break;
    }
    std::swap(heap_of_queues_of_events_ordered_by_fd_[parent_index],
              heap_of_queues_of_events_ordered_by_fd_[current_index]);
    current_index = parent_index;
  }
}

}  // namespace orbit_linux_tracing
