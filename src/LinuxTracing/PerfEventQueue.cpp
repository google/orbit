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
    unordered_events_priority_queue_.push(std::move(event));

  } else if (auto queue_it = ordered_queues_by_fd_.find(origin_fd);
             queue_it != ordered_queues_by_fd_.end()) {
    const std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>& queue = queue_it->second;

    CHECK(!queue->empty());
    // Fundamental assumption: events from the same file descriptor come already in order.
    CHECK(event->GetTimestamp() >= queue->back()->GetTimestamp());
    queue->push(std::move(event));

  } else {
    queue_it = ordered_queues_by_fd_
                   .emplace(origin_fd, std::make_unique<std::queue<std::unique_ptr<PerfEvent>>>())
                   .first;
    const std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>& queue = queue_it->second;

    queue->push(std::move(event));
    ordered_queues_heap_.emplace_back(queue.get());
    MoveUpBackOfOrderedQueuesHeap();
  }
}

bool PerfEventQueue::HasEvent() const {
  return !ordered_queues_heap_.empty() || !unordered_events_priority_queue_.empty();
}

PerfEvent* PerfEventQueue::TopEvent() {
  // As we effectively have two priority queues, get the older event between the two events at the
  // top of the two queues.
  PerfEvent* top_event = nullptr;
  if (!unordered_events_priority_queue_.empty()) {
    top_event = unordered_events_priority_queue_.top().get();
  }
  if (!ordered_queues_heap_.empty() &&
      (top_event == nullptr ||
       ordered_queues_heap_.front()->front()->GetTimestamp() < top_event->GetTimestamp())) {
    top_event = ordered_queues_heap_.front()->front().get();
  }
  CHECK(top_event != nullptr);
  return top_event;
}

std::unique_ptr<PerfEvent> PerfEventQueue::PopEvent() {
  if (!unordered_events_priority_queue_.empty() &&
      (ordered_queues_heap_.empty() || unordered_events_priority_queue_.top()->GetTimestamp() <
                                           ordered_queues_heap_.front()->front()->GetTimestamp())) {
    // The oldest event is at the top of the priority queue holding the events that cannot be
    // assumed sorted in any ring buffer.
    std::unique_ptr<PerfEvent> top_event =
        std::move(const_cast<std::unique_ptr<PerfEvent>&>(unordered_events_priority_queue_.top()));
    unordered_events_priority_queue_.pop();
    return top_event;
  }

  std::queue<std::unique_ptr<PerfEvent>>* top_queue = ordered_queues_heap_.front();
  std::unique_ptr<PerfEvent> top_event = std::move(top_queue->front());
  top_queue->pop();

  if (top_queue->empty()) {
    int top_fd = top_event->GetOrderedInFileDescriptor();
    ordered_queues_by_fd_.erase(top_fd);
    std::swap(ordered_queues_heap_.front(), ordered_queues_heap_.back());
    ordered_queues_heap_.pop_back();
    MoveDownFrontOfOrderedQueuesHeap();
  } else {
    MoveDownFrontOfOrderedQueuesHeap();
  }

  return top_event;
}

void PerfEventQueue::MoveDownFrontOfOrderedQueuesHeap() {
  if (ordered_queues_heap_.empty()) {
    return;
  }

  size_t current_index = 0;
  size_t new_index;
  while (true) {
    new_index = current_index;
    size_t left_index = current_index * 2 + 1;
    size_t right_index = current_index * 2 + 2;
    if (left_index < ordered_queues_heap_.size() &&
        ordered_queues_heap_[left_index]->front()->GetTimestamp() <
            ordered_queues_heap_[new_index]->front()->GetTimestamp()) {
      new_index = left_index;
    }
    if (right_index < ordered_queues_heap_.size() &&
        ordered_queues_heap_[right_index]->front()->GetTimestamp() <
            ordered_queues_heap_[new_index]->front()->GetTimestamp()) {
      new_index = right_index;
    }
    if (new_index != current_index) {
      std::swap(ordered_queues_heap_[new_index], ordered_queues_heap_[current_index]);
      current_index = new_index;
    } else {
      break;
    }
  }
}

void PerfEventQueue::MoveUpBackOfOrderedQueuesHeap() {
  if (ordered_queues_heap_.empty()) {
    return;
  }

  size_t current_index = ordered_queues_heap_.size() - 1;
  while (current_index > 0) {
    size_t parent_index = (current_index - 1) / 2;
    if (ordered_queues_heap_[parent_index]->front()->GetTimestamp() <=
        ordered_queues_heap_[current_index]->front()->GetTimestamp()) {
      break;
    }
    std::swap(ordered_queues_heap_[parent_index], ordered_queues_heap_[current_index]);
    current_index = parent_index;
  }
}

}  // namespace orbit_linux_tracing
