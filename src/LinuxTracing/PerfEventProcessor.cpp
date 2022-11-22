// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventProcessor.h"

#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "PerfEvent.h"

namespace orbit_linux_tracing {

void PerfEventProcessor::AddEvent(PerfEvent&& event) {
  const uint64_t timestamp = event.timestamp;
  if (last_processed_timestamp_ns_ > 0 && timestamp < last_processed_timestamp_ns_) {
    if (discarded_out_of_order_counter_ != nullptr) {
      ++(*discarded_out_of_order_counter_);
    }

    std::optional<DiscardedPerfEvent> discarded_perf_event = HandleOutOfOrderEvent(timestamp);
    if (discarded_perf_event.has_value()) {
      event_queue_.PushEvent(std::move(discarded_perf_event.value()));
    }
    return;
  }
  event_queue_.PushEvent(std::move(event));
}

// When a PerfEvent gets discarded, this method possibly generates a DiscardedPerfEvent in its
// place. In particular, it reports a DiscardedPerfEvent that covers the time range between the
// discarded event's timestamp and last_processed_timestamp_ns_.
// A DiscardedPerfEvent is not always produced, though. If the time range of the DiscardedPerfEvent
// that should be produced is entirely contained in the time range of the last DiscardedPerfEvent
// (if any), no DiscardedPerfEvent is produced. This seems very specific (see the if-else chain
// below), but is instead by far the most common case, as usually discarded events are caused by a
// burst of events coming (late) all one after the other and all from the same ring buffer, hence
// generally in order. So even from a considerable amount of discarded PerfEvents result only few
// DiscardedPerfEvents.
std::optional<DiscardedPerfEvent> PerfEventProcessor::HandleOutOfOrderEvent(
    uint64_t event_timestamp_ns) {
  const uint64_t discarded_begin = event_timestamp_ns;
  const uint64_t discarded_end = last_processed_timestamp_ns_;

  std::optional<DiscardedPerfEvent> optional_discarded_event = std::nullopt;

  ORBIT_CHECK(discarded_end >= last_discarded_end_);
  if (discarded_end == last_discarded_end_ && discarded_begin < last_discarded_begin_) {
    optional_discarded_event = optional_discarded_event = DiscardedPerfEvent{
        .timestamp = discarded_end,
        .data = {.begin_timestamp_ns = discarded_begin},
    };
    last_discarded_begin_ = discarded_begin;
  } else if (discarded_end == last_discarded_end_ && discarded_begin >= last_discarded_begin_) {
    // This is the only case that doesn't generate a DiscardedPerfEvent.
  } else if (discarded_end > last_discarded_end_ && discarded_begin < last_discarded_begin_) {
    optional_discarded_event = DiscardedPerfEvent{
        .timestamp = discarded_end,
        .data = {.begin_timestamp_ns = discarded_begin},
    };
    last_discarded_begin_ = discarded_begin;
  } else if (discarded_end > last_discarded_end_ && discarded_begin <= last_discarded_end_) {
    optional_discarded_event = DiscardedPerfEvent{
        .timestamp = discarded_end,
        .data = {.begin_timestamp_ns = discarded_begin},
    };
    // Don't update last_discarded_begin_.
  } else if (discarded_end > last_discarded_end_ && discarded_begin > last_discarded_end_) {
    optional_discarded_event = DiscardedPerfEvent{
        .timestamp = discarded_end,
        .data = {.begin_timestamp_ns = discarded_begin},
    };
    last_discarded_begin_ = discarded_begin;
  } else {
    ORBIT_UNREACHABLE();
  }

  last_discarded_end_ = discarded_end;

  // The timestamp of DiscardedPerfEvent is last_processed_timestamp_ns_, so the event can be
  // processed and it will probably be processed next.
  return optional_discarded_event;
}

void PerfEventProcessor::ProcessAllEvents() {
  ORBIT_CHECK(!visitors_.empty());
  while (event_queue_.HasEvent()) {
    const PerfEvent& event = event_queue_.TopEvent();
    // Events are guaranteed to be processed in order of timestamp
    // as out-of-order events are discarded in AddEvent.
    ORBIT_CHECK(event.timestamp >= last_processed_timestamp_ns_);
    last_processed_timestamp_ns_ = event.timestamp;
    for (PerfEventVisitor* visitor : visitors_) {
      event.Accept(visitor);
    }
    event_queue_.PopEvent();
  }
}

void PerfEventProcessor::ProcessOldEvents() {
  ORBIT_CHECK(!visitors_.empty());
  const uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  while (event_queue_.HasEvent()) {
    const PerfEvent& event = event_queue_.TopEvent();
    const uint64_t timestamp = event.timestamp;

    // Do not read the most recent events as out-of-order events could (and will) arrive.
    if (timestamp + kProcessingDelayMs * 1'000'000 >= current_timestamp_ns) {
      break;
    }
    // Events are guaranteed to be processed in order of timestamp
    // as out-of-order events are discarded in AddEvent.
    ORBIT_CHECK(timestamp >= last_processed_timestamp_ns_);
    last_processed_timestamp_ns_ = timestamp;

    for (PerfEventVisitor* visitor : visitors_) {
      event.Accept(visitor);
    }
    event_queue_.PopEvent();
  }
}

}  // namespace orbit_linux_tracing
