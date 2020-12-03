// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_QUEUE_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_QUEUE_H_

#include <queue>

#include "PerfEvent.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

// This class implements a data structure that holds a large number of different
// perf_event_open records coming from multiple ring buffers, and allows reading
// them in order (oldest first).
// Instead of keeping a single priority queue with all the events to process,
// on which push/pop operations would be logarithmic in the number of events,
// we leverage the fact that events coming from the same perf_event_open ring
// buffer are already sorted. We then keep a priority queue of queues, where
// the events in each queue come from the same ring buffer. Whenever an event
// is removed from a queue, we need to move such queue down the priority
// queue. As std::priority_queue does not support decreasing the priority of
// an element, we achieve this by removing and re-inserting.
// In order to be able to add an event to a queue, we also need to maintain
// the association between a queue and its ring buffer. We use the file
// descriptor used to read from the ring buffer as identifier for a ring
// buffer. Keeping this association is what the pairs and the map are for.
// TODO: Implement a custom priority queue that supports decreasing the
//  priority.
class PerfEventQueue {
 public:
  void PushEvent(int origin_fd, std::unique_ptr<PerfEvent> event);
  bool HasEvent();
  PerfEvent* TopEvent();
  std::unique_ptr<PerfEvent> PopEvent();

 private:
  // Comparator for the priority queue: pop will return the queue associated
  // with the file descriptor from which the oldest event still to process
  // originated.
  struct QueueFrontTimestampReverseCompare {
    bool operator()(
        const std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>>& lhs,
        const std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>>& rhs) {
      return lhs.second->front()->GetTimestamp() > rhs.second->front()->GetTimestamp();
    }
  };

  std::priority_queue<
      std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>>,
      std::vector<std::pair<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>>>,
      QueueFrontTimestampReverseCompare>
      event_queues_queue_{};
  absl::flat_hash_map<int, std::shared_ptr<std::queue<std::unique_ptr<PerfEvent>>>>
      fd_event_queues_{};
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_QUEUE_H_
