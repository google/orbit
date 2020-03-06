#include "PerfEventProcessor2.h"

#include <cassert>
#include <memory>
#include <queue>

#include "Logging.h"
#include "PerfEvent.h"
#include "Utils.h"

namespace LinuxTracing {

void PerfEventQueue::PushEvent(int origin_fd,
                               std::unique_ptr<PerfEvent> event) {
  if (fd_event_queues_.count(origin_fd) > 0) {
    std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>> event_queue =
        fd_event_queues_.at(origin_fd);
    assert(!event_queue->empty());
    // Fundamental assumption: events from the same file descriptor come already
    // in order.
    assert(event->GetTimestamp() >= event_queue->front()->GetTimestamp());
    event_queue->push(std::move(event));
  } else {
    auto event_queue =
        std::make_shared<std::queue<std::unique_ptr<PerfEvent>>>();
    fd_event_queues_.insert(std::make_pair(origin_fd, event_queue));
    event_queue->push(std::move(event));
    event_queues_queue_.push(std::make_pair(origin_fd, event_queue));
  }
}

bool PerfEventQueue::HasEvent() { return !event_queues_queue_.empty(); }

PerfEvent* PerfEventQueue::TopEvent() {
  return event_queues_queue_.top().second->front().get();
}

std::unique_ptr<PerfEvent> PerfEventQueue::PopEvent() {
  std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>>
      top_fd_queue = event_queues_queue_.top();
  event_queues_queue_.pop();
  const int& top_fd = top_fd_queue.first;
  std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>& top_queue =
      top_fd_queue.second;

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

void PerfEventProcessor2::AddEvent(int origin_fd,
                                   std::unique_ptr<PerfEvent> event) {
#ifndef NDEBUG
  if (last_processed_timestamp_ > 0 &&
      event->GetTimestamp() <
          last_processed_timestamp_ - PROCESSING_DELAY_MS * 1'000'000) {
    ERROR("Processed an event out of order");
  }
#endif
  event_queue_.PushEvent(origin_fd, std::move(event));
}

void PerfEventProcessor2::ProcessAllEvents() {
  while (event_queue_.HasEvent()) {
    std::unique_ptr<PerfEvent> event = event_queue_.PopEvent();
    event->Accept(visitor_.get());
#ifndef NDEBUG
    last_processed_timestamp_ = event->GetTimestamp();
#endif
  }
}

void PerfEventProcessor2::ProcessOldEvents() {
  uint64_t max_timestamp = MonotonicTimestampNs();

  while (event_queue_.HasEvent()) {
    PerfEvent* event = event_queue_.TopEvent();

    // Do not read the most recent events as out-of-order events could arrive.
    if (event->GetTimestamp() + PROCESSING_DELAY_MS * 1'000'000 >=
        max_timestamp) {
      break;
    }

    event->Accept(visitor_.get());
#ifndef NDEBUG
    last_processed_timestamp_ = event->GetTimestamp();
#endif
    event_queue_.PopEvent();
  }
}

}  // namespace LinuxTracing
