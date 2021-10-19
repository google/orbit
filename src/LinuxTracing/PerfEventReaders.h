// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_READERS_H_
#define LINUX_TRACING_PERF_EVENT_READERS_H_

#include <linux/perf_event.h>
#include <stdint.h>
#include <sys/types.h>

#include "PerfEvent.h"
#include "PerfEventRecords.h"
#include "PerfEventRingBuffer.h"

namespace orbit_linux_tracing {

// Helper functions for reads from a perf_event_open ring buffer that require
// more complex operations than simply copying an entire perf_event_open record.

// This function reads sample_id, which is always the last field
// in the perf event record unless it is PERF_RECORD_SAMPLE.
void ReadPerfSampleIdAll(PerfEventRingBuffer* ring_buffer, const perf_event_header& header,
                         perf_event_sample_id_tid_time_streamid_cpu* sample_id);

uint64_t ReadSampleRecordTime(PerfEventRingBuffer* ring_buffer);

uint64_t ReadSampleRecordStreamId(PerfEventRingBuffer* ring_buffer);

pid_t ReadSampleRecordPid(PerfEventRingBuffer* ring_buffer);

uint64_t ReadThrottleUnthrottleRecordTime(PerfEventRingBuffer* ring_buffer);

PerfEvent ConsumeStackSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                      const perf_event_header& header);

PerfEvent ConsumeCallchainSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                          const perf_event_header& header);

PerfEvent ConsumeGenericTracepointPerfEvent(PerfEventRingBuffer* ring_buffer,
                                            const perf_event_header& header);

PerfEvent ConsumeMmapPerfEvent(PerfEventRingBuffer* ring_buffer, const perf_event_header& header);

PerfEvent ConsumeAmdgpuCsIoctlPerfEvent(PerfEventRingBuffer* ring_buffer,
                                        const perf_event_header& header);

PerfEvent ConsumeAmdgpuSchedRunJobPerfEvent(PerfEventRingBuffer* ring_buffer,
                                            const perf_event_header& header);

PerfEvent ConsumeDmaFenceSignaledPerfEvent(PerfEventRingBuffer* ring_buffer,
                                           const perf_event_header& header);
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_READERS_H_
