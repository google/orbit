// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventQueue.h"

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
    queues_heap_.emplace_back(queue.get());
    MoveUpHeapBack();
  }
}

bool PerfEventQueue::HasEvent() { return !queues_heap_.empty(); }

PerfEvent* PerfEventQueue::TopEvent() { return queues_heap_.front()->front().get(); }

std::unique_ptr<PerfEvent> PerfEventQueue::PopEvent() {
  std::queue<std::unique_ptr<PerfEvent>>* top_queue = queues_heap_.front();
  std::unique_ptr<PerfEvent> top_event = std::move(top_queue->front());
  top_queue->pop();

  if (top_queue->empty()) {
    int top_fd = top_event->GetOriginFileDescriptor();
    queues_.erase(top_fd);
    std::swap(queues_heap_.front(), queues_heap_.back());
    queues_heap_.pop_back();
    MoveDownHeapFront();
  } else {
    MoveDownHeapFront();
  }

  return top_event;
}

void PerfEventQueue::MoveDownHeapFront() {
  if (queues_heap_.empty()) {
    return;
  }

  size_t current_index = 0;
  size_t new_index;
  while (true) {
    new_index = current_index;
    size_t left_index = current_index * 2 + 1;
    size_t right_index = current_index * 2 + 2;
    if (left_index < queues_heap_.size() && queues_heap_[left_index]->front()->GetTimestamp() <
                                                queues_heap_[new_index]->front()->GetTimestamp()) {
      new_index = left_index;
    }
    if (right_index < queues_heap_.size() && queues_heap_[right_index]->front()->GetTimestamp() <
                                                 queues_heap_[new_index]->front()->GetTimestamp()) {
      new_index = right_index;
    }
    if (new_index != current_index) {
      std::swap(queues_heap_[new_index], queues_heap_[current_index]);
      current_index = new_index;
    } else {
      break;
    }
  }
}

void PerfEventQueue::MoveUpHeapBack() {
  if (queues_heap_.empty()) {
    return;
  }

  size_t current_index = queues_heap_.size() - 1;
  while (current_index > 0) {
    size_t parent_index = (current_index - 1) / 2;
    if (queues_heap_[parent_index]->front()->GetTimestamp() <=
        queues_heap_[current_index]->front()->GetTimestamp()) {
      break;
    }
    std::swap(queues_heap_[parent_index], queues_heap_[current_index]);
    current_index = parent_index;
  }
}

}  // namespace LinuxTracing
