//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "LinuxPerfEventProcessor.h"

#include <queue>

#include "Profiling.h"

void PerfEventQueue::PushEvent(int origin_fd,
                               std::unique_ptr<LinuxPerfEvent> event) {
  if (fd_event_queues_.count(origin_fd) > 0) {
    std::shared_ptr<std::queue<std::unique_ptr<LinuxPerfEvent>>> event_queue =
        fd_event_queues_.at(origin_fd);
    assert(!event_queue->empty());
    // Fundamental assumption: events from the same file descriptor come already
    // in order.
    assert(event->Timestamp() >= event_queue->front()->Timestamp());
    event_queue->push(std::move(event));
  } else {
    auto event_queue =
        std::make_shared<std::queue<std::unique_ptr<LinuxPerfEvent>>>();
    fd_event_queues_.insert(std::make_pair(origin_fd, event_queue));
    event_queue->push(std::move(event));
    event_queues_queue_.push(std::make_pair(origin_fd, event_queue));
  }
}

bool PerfEventQueue::HasEvent() { return !event_queues_queue_.empty(); }

LinuxPerfEvent* PerfEventQueue::TopEvent() {
  return event_queues_queue_.top().second->front().get();
}

std::unique_ptr<LinuxPerfEvent> PerfEventQueue::PopEvent() {
  std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<LinuxPerfEvent>>>>
      top_fd_queue = event_queues_queue_.top();
  event_queues_queue_.pop();
  const int& top_fd = top_fd_queue.first;
  std::shared_ptr<std::queue<std::unique_ptr<LinuxPerfEvent>>>& top_queue =
      top_fd_queue.second;

  std::unique_ptr<LinuxPerfEvent> top_event = std::move(top_queue->front());
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

void LinuxPerfEventProcessor::AddEvent(int origin_fd,
                                       std::unique_ptr<LinuxPerfEvent> event) {
#ifndef NDEBUG
  if (last_processed_timestamp_ > 0 &&
      event->Timestamp() <
          last_processed_timestamp_ - PROCESSING_DELAY_MS * 1'000'000) {
    PRINT("Error: processed an event out of order.\n");
  }
#endif
  event_queue_.PushEvent(origin_fd, std::move(event));
}

void LinuxPerfEventProcessor::ProcessAllEvents() {
  while (event_queue_.HasEvent()) {
    std::unique_ptr<LinuxPerfEvent> event = event_queue_.PopEvent();
    event->accept(visitor_.get());
#ifndef NDEBUG
    last_processed_timestamp_ = event->Timestamp();
#endif
  }
}

void LinuxPerfEventProcessor::ProcessOldEvents() {
  uint64_t max_timestamp = OrbitTicks(CLOCK_MONOTONIC);

  while (event_queue_.HasEvent()) {
    LinuxPerfEvent* event = event_queue_.TopEvent();

    // Do not read the most recent events as out-of-order events could arrive.
    if (event->Timestamp() + PROCESSING_DELAY_MS * 1'000'000 >= max_timestamp) {
      break;
    }

    event->accept(visitor_.get());
#ifndef NDEBUG
    last_processed_timestamp_ = event->Timestamp();
#endif
    event_queue_.PopEvent();
  }
}
