// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_PROCESSOR_H_
#define LINUX_TRACING_PERF_EVENT_PROCESSOR_H_

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

#include "PerfEvent.h"
#include "PerfEventQueue.h"
#include "PerfEventVisitor.h"

namespace orbit_linux_tracing {

// This class receives perf_event_open events coming from several ring buffers
// and processes them in order according to their timestamps.
// Its implementation builds on the assumption that we never expect events with
// a timestamp older than kProcessingDelayMs to be added. By not processing
// events that are not older than this delay, we will never process events out
// of order.
class PerfEventProcessor {
 public:
  void AddEvent(std::unique_ptr<PerfEvent> event);

  void ProcessAllEvents();

  void ProcessOldEvents();

  void AddVisitor(PerfEventVisitor* visitor) { visitors_.push_back(visitor); }

  void ClearVisitors() { visitors_.clear(); }

  void SetDiscardedOutOfOrderCounter(std::atomic<uint64_t>* discarded_out_of_order_counter) {
    discarded_out_of_order_counter_ = discarded_out_of_order_counter;
  }

 private:
  // Do not process events that are more recent than 0.1 seconds. There could be
  // events coming out of order as they are read from different perf_event_open
  // ring buffers and this ensure that all events are processed in the correct
  // order.
  static constexpr uint64_t kProcessingDelayMs = 100;
  uint64_t last_processed_timestamp_ns_ = 0;
  std::atomic<uint64_t>* discarded_out_of_order_counter_ = nullptr;

  PerfEventQueue event_queue_;
  std::vector<PerfEventVisitor*> visitors_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_PROCESSOR_H_
