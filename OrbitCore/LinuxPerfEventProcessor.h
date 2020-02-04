//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#ifndef ORBIT_CORE_LINUX_PERF_EVENT_PROCESSOR_H_
#define ORBIT_CORE_LINUX_PERF_EVENT_PROCESSOR_H_

#include <ctime>
#include <queue>

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "PrintVar.h"

// A comparator used for the priority queue, such that pop/top will
// always return the oldest event in the queue.
class TimestampReverseCompare {
 public:
  bool operator()(const std::unique_ptr<LinuxPerfEvent>& lhs,
                  const std::unique_ptr<LinuxPerfEvent>& rhs) {
    return lhs->Timestamp() > rhs->Timestamp();
  }
};

// This class implements a data structure that holds a large number of different
// perf_event_open records coming from multiple ring buffers, and allows reading
// them in order (oldest first). In other words, it synchronizes events from all
// ring buffers according to their timestamps.
// Its implementation builds on the assumption that we never expect events with
// a timestamp older than PROCESSING_DELAY_MS to be added. By not processing
// events that are not older than this delay, we will never process events out
// of order.
class LinuxPerfEventProcessor {
 public:
  // Do not process events that are more recent than 0.1 seconds. There could be
  // events coming out of order as they are read from different perf_event_open
  // ring buffers and this ensure that all events are processed in the correct
  // order.
  static constexpr uint64_t PROCESSING_DELAY_MS = 100;

  explicit LinuxPerfEventProcessor(
      std::unique_ptr<LinuxPerfEventVisitor> visitor)
      : visitor_(std::move(visitor)) {}

  void Push(std::unique_ptr<LinuxPerfEvent> event);

  void ProcessAllEvents();

  void ProcessOldEvents();

 private:
  // TODO: Inserting into this buffer is logarithmic in its size. However, all
  //  events within one ring buffer are already sorted. We could instead
  //  implement a priority queue of ring buffers and be logarithmic in the
  //  number of buffers. This would require moving a ring buffer down the heap
  //  once an event has been processed, by removing-and-readding or with a
  //  custom priority queue.
  std::priority_queue<std::unique_ptr<LinuxPerfEvent>,
                      std::vector<std::unique_ptr<LinuxPerfEvent>>,
                      TimestampReverseCompare>
      event_queue_;

  std::unique_ptr<LinuxPerfEventVisitor> visitor_;

#ifndef NDEBUG
  uint64_t last_processed_timestamp_ = 0;
#endif
};

#endif  // ORBIT_CORE_LINUX_PERF_EVENT_PROCESSOR_H_
