// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_READERS_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_READERS_H_

#include "PerfEvent.h"
#include "PerfEventRingBuffer.h"

namespace LinuxTracing {

// Helper functions for reads from a perf_event_open ring buffer that require
// more complex operations than simply copying an entire perf_event_open record.
pid_t ReadMmapRecordPid(PerfEventRingBuffer* ring_buffer);

uint64_t ReadSampleRecordStreamId(PerfEventRingBuffer* ring_buffer);

pid_t ReadSampleRecordPid(PerfEventRingBuffer* ring_buffer);

std::unique_ptr<StackSamplePerfEvent> ConsumeStackSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                                                  const perf_event_header& header);

std::unique_ptr<CallchainSamplePerfEvent> ConsumeCallchainSamplePerfEvent(
    PerfEventRingBuffer* ring_buffer, const perf_event_header& header);

std::unique_ptr<GenericTracepointPerfEvent> ConsumeGenericTracepointPerfEvent(
    PerfEventRingBuffer* ring_buffer, const perf_event_header& header);

template <typename T, typename = std::enable_if_t<std::is_base_of_v<TracepointPerfEvent, T>>>
std::unique_ptr<T> ConsumeTracepointPerfEvent(PerfEventRingBuffer* ring_buffer,
                                              const perf_event_header& header) {
  uint32_t tracepoint_size;
  ring_buffer->ReadValueAtOffset(&tracepoint_size, offsetof(perf_event_raw_sample_fixed, size));
  auto event = std::make_unique<T>(tracepoint_size);
  ring_buffer->ReadRawAtOffset(&event->ring_buffer_record, 0, sizeof(perf_event_raw_sample_fixed));
  ring_buffer->ReadRawAtOffset(&event->tracepoint_data[0],
                               offsetof(perf_event_raw_sample_fixed, size) + sizeof(uint32_t),
                               tracepoint_size);
  ring_buffer->SkipRecord(header);
  return event;
}

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_READERS_H_
