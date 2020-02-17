//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "LinuxPerfEventProcessor.h"

#include <queue>

#include "Profiling.h"

void LinuxPerfEventProcessor::AddEvent(int origin_fd,
                                       std::unique_ptr<LinuxPerfEvent> event) {
#ifndef NDEBUG
  if (last_processed_timestamp_ > 0 &&
      event->Timestamp() <
          last_processed_timestamp_ - PROCESSING_DELAY_MS * 1'000'000) {
    PRINT("Error: processed an event out of order.\n");
  }
#endif

  event_queue_.push(std::move(event));
}

void LinuxPerfEventProcessor::ProcessAllEvents() {
  while (!event_queue_.empty()) {
    LinuxPerfEvent* event = event_queue_.top().get();
    event->accept(visitor_.get());

#ifndef NDEBUG
    last_processed_timestamp_ = event->Timestamp();
#endif

    event_queue_.pop();
  }
}

void LinuxPerfEventProcessor::ProcessOldEvents() {
  uint64_t max_timestamp = OrbitTicks(CLOCK_MONOTONIC);

  while (!event_queue_.empty()) {
    LinuxPerfEvent* event = event_queue_.top().get();

    // Do not read the most recent events are out-of-order events could arrive.
    if (event->Timestamp() + PROCESSING_DELAY_MS * 1'000'000 >= max_timestamp) {
      break;
    }
    event->accept(visitor_.get());

#ifndef NDEBUG
    last_processed_timestamp_ = event->Timestamp();
#endif

    event_queue_.pop();
  }
}
