//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "PrintVar.h"

#include <queue>

// A comparator used for the priority queue, such that pop/top will
// always return the oldest event in the queue.
class TimestampCompare {
 public:
  bool operator()(const std::unique_ptr<LinuxPerfEvent>& a_LHS,
                  const std::unique_ptr<LinuxPerfEvent>& a_RHS) {
    return a_LHS->Timestamp() > a_RHS->Timestamp();
  }
};

// This class implements a data structure that holds a large number of
//  possible DIFFERENT entries (events) of various perf ring buffers,
//  and allows reading them in a garantueed order (oldest first).
//  So it synchs the events from all the buffers according their timestamps.
// Its implementation builds on the following assumption:
//  We never expect events with a timestamp older than DELAY_IN_NS to be
//  added.
//  So we will stop processing at the first event that is close (according to
//  the delay) to the latest timestamp seen. If we only touch events that are
//  old enough, we will never process events out of order.

// TODO: instead of having unique pointers inside the priority_queue,
//  we could have a queue of LinuxPerfEvent objects. This would give use
//  better caching, but we need to copy more data, when rearanging the
//  heap on insertion.
// TODO: inserting to this buffer is log(num events). However, all events
//  within one ringbuffer are already sorted by the timestamp. So we could
//  implement a priority_queue of ringbuffers, to get log(num ringbuffers).
//  However, we would need to be very carefull, as the timestamps of the
//  buffers change over time, and there might be buffers, that do not have
//  a new event yet, but might have later.
class LinuxPerfEventProcessor {
 public:
  // While processing, we do not touch the events with a timestamp less
  // than 1/10 sec smaller than the most recent one in the queue.
  // This way we can ensure, that all events (from different sources)
  // are processed in the correct order.
  const uint64_t DELAY_IN_NS = 100000000 /*ns*/;

  LinuxPerfEventProcessor(std::unique_ptr<LinuxPerfEventVisitor> a_Visitor)
      : m_Visitor(std::move(a_Visitor)) {}

  void Push(std::unique_ptr<LinuxPerfEvent> a_Event);

  void ProcessAll();

  void ProcessTillOffset();

 private:
  std::priority_queue<std::unique_ptr<LinuxPerfEvent>,
                      std::vector<std::unique_ptr<LinuxPerfEvent>>,
                      TimestampCompare>
      m_EventQueue;

  std::unique_ptr<LinuxPerfEventVisitor> m_Visitor;

  uint64_t m_MaxTimestamp = 0;
#ifndef NDEBUG
  uint64_t m_LastProcessTimestamp = 0;
#endif
};