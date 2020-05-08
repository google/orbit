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

std::unique_ptr<StackSamplePerfEvent> ConsumeStackSamplePerfEvent(
    PerfEventRingBuffer* ring_buffer, const perf_event_header& header);

std::unique_ptr<CallchainSamplePerfEvent> ConsumeCallchainSamplePerfEvent(
    PerfEventRingBuffer* ring_buffer, const perf_event_header& header);

std::unique_ptr<PerfEventSampleRaw> ConsumeSampleRaw(
    PerfEventRingBuffer* ring_buffer, const perf_event_header& header);

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_READERS_H_
