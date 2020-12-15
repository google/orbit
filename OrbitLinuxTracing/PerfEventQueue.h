// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_QUEUE_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_QUEUE_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>

#include <memory>
#include <queue>
#include <vector>

#include "PerfEvent.h"

namespace orbit_linux_tracing {

// This class implements a data structure that holds a large number of different perf_event_open
// records coming from multiple ring buffers, and allows reading them in order (oldest first).
//
// Instead of keeping a single priority queue with all the events to process, on which push/pop
// operations would be logarithmic in the number of events, we leverage the fact that events coming
// from the same perf_event_open ring buffer are already sorted. We then keep a priority queue of
// queues, where the events in each queue come from the same ring buffer. Whenever an event is
// removed from a queue, we need to move such queue down the priority queue.
//
// In order to be able to add an event to a queue, we also need to maintain the association between
// a queue and its ring buffer, which is what the map is for. We use the file descriptor used to
// read from the ring buffer as identifier for a ring buffer.
class PerfEventQueue {
 public:
  void PushEvent(std::unique_ptr<PerfEvent> event);
  [[nodiscard]] bool HasEvent();
  [[nodiscard]] PerfEvent* TopEvent();
  std::unique_ptr<PerfEvent> PopEvent();

 private:
  // Floats down the element at the top of the heap to its correct place. Used when the key
  // of the top element changes, or as part of the process of removing the top element.
  void MoveDownHeapFront();
  // Floats up an element that it is know should be further up in the heap. Used on insertion.
  void MoveUpHeapBack();

 private:
  absl::flat_hash_map<int, std::unique_ptr<std::queue<std::unique_ptr<PerfEvent>>>> queues_;
  std::vector<std::queue<std::unique_ptr<PerfEvent>>*> queues_heap_;
};

}  // namespace orbit_linux_tracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_QUEUE_H_
