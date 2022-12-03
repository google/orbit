// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_QUEUE_H_
#define LINUX_TRACING_PERF_EVENT_QUEUE_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "PerfEvent.h"
#include "PerfEventOrderedStream.h"

namespace orbit_linux_tracing {

// This class implements a data structure that holds a large number of different PerfEvents coming
// from multiple sources, e.g., perf_event_open records coming from multiple ring buffers, and
// allows reading them in order (oldest first).
//
// Instead of keeping a single priority queue with all the events to process, on which push/pop
// operations would be logarithmic in the number of events, we leverage the fact that some streams
// of events are known to be already sorted; for example, most perf_event_open records coming from
// the same perf_event_open ring buffer are already sorted. We then keep a priority queue of queues,
// where the events in each queue come from the same sorted stream, identified by matching instances
// of PerfEventOrderedStream. Whenever an event is removed from a queue, we need to move such queue
// down the priority queue.
//
// In order to be able to add an event to a queue, we also need to maintain the association between
// a queue and its sorted stream, which is what the map is for. We use the PerfEventOrderedStream as
// key.
//
// Some events, though, are known to come out of order even in relation to other events in the same
// perf_event_open ring buffer (e.g., dma_fence_signaled). For those cases, use an additional single
// std::priority_queue.
class PerfEventQueue {
 public:
  void PushEvent(PerfEvent&& event);
  [[nodiscard]] bool HasEvent() const;
  [[nodiscard]] const PerfEvent& TopEvent();
  void PopEvent();

 private:
  // Floats down the element at the top of the ordered_queues_heap_ to its correct place. Used when
  // the key of the top element changes, or as part of the process of removing the top element.
  void MoveDownFrontOfHeapOfQueues();
  // Floats up an element that it is know should be further up in the heap. Used on insertion.
  void MoveUpBackOfHeapOfQueues();

  // This vector holds the heap of the queues each of which holds events coming from the same
  // stream of events already in order by timestamp.
  std::vector<std::queue<PerfEvent>*> heap_of_queues_of_events_ordered_in_stream_;
  // This map keeps the association between an ordered stream of events and the ordered queue of
  // events coming from that stream.
  absl::flat_hash_map<PerfEventOrderedStream, std::unique_ptr<std::queue<PerfEvent>>>
      queues_of_events_ordered_in_stream_;

  static constexpr auto kPerfEventReverseTimestampCompare =
      [](const PerfEvent& lhs, const PerfEvent& rhs) { return lhs.timestamp > rhs.timestamp; };
  // This priority queue holds all those events that cannot be assumed already sorted in a specific
  // stream. All such events are simply sorted by the priority queue by increasing timestamp.
  std::priority_queue<PerfEvent, std::vector<PerfEvent>,
                      std::function<bool(const PerfEvent&, const PerfEvent&)>>
      priority_queue_of_events_not_ordered_in_stream_{kPerfEventReverseTimestampCompare};
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_QUEUE_H_
