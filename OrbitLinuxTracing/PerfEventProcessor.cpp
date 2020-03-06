#include "PerfEventProcessor.h"

#include <queue>

#include "Logging.h"
#include "Utils.h"

namespace LinuxTracing {

void PerfEventProcessor::AddEvent(int origin_fd,
                                  std::unique_ptr<PerfEvent> event) {
#ifndef NDEBUG
  if (last_processed_timestamp_ > 0 &&
      event->GetTimestamp() <
          last_processed_timestamp_ - PROCESSING_DELAY_MS * 1'000'000) {
    ERROR("Processed an event out of order");
  }
#endif

  event_queue_.push(std::move(event));
}

void PerfEventProcessor::ProcessAllEvents() {
  while (!event_queue_.empty()) {
    PerfEvent* event = event_queue_.top().get();
    event->Accept(visitor_.get());

#ifndef NDEBUG
    last_processed_timestamp_ = event->GetTimestamp();
#endif

    event_queue_.pop();
  }
}

void PerfEventProcessor::ProcessOldEvents() {
  uint64_t max_timestamp = MonotonicTimestampNs();

  while (!event_queue_.empty()) {
    PerfEvent* event = event_queue_.top().get();

    // Do not read the most recent events are out-of-order events could arrive.
    if (event->GetTimestamp() + PROCESSING_DELAY_MS * 1'000'000 >=
        max_timestamp) {
      break;
    }
    event->Accept(visitor_.get());

#ifndef NDEBUG
    last_processed_timestamp_ = event->GetTimestamp();
#endif

    event_queue_.pop();
  }
}

}  // namespace LinuxTracing
