// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventProcessor.h"

#include <memory>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "PerfEvent.h"

namespace orbit_linux_tracing {

void PerfEventProcessor::AddEvent(std::unique_ptr<PerfEvent> event) {
  if (last_processed_timestamp_ns_ > 0 && event->GetTimestamp() < last_processed_timestamp_ns_) {
    if (discarded_out_of_order_counter_ != nullptr) {
      ++(*discarded_out_of_order_counter_);
    }
    return;
  }
  event_queue_.PushEvent(std::move(event));
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

}  // namespace orbit_linux_tracing
