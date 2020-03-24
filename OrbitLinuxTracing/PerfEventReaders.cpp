#include "PerfEventReaders.h"

#include <OrbitBase/Logging.h>

#include "PerfEventRecords.h"
#include "PerfEventRingBuffer.h"

namespace LinuxTracing {

pid_t ReadMmapRecordPid(PerfEventRingBuffer* ring_buffer) {
  // Mmap records have the following layout:
  // struct {
  //   struct perf_event_header header;
  //   u32    pid, tid;
  //   u64    addr;
  //   u64    len;
  //   u64    pgoff;
  //   char   filename[];
  //   struct sample_id sample_id; /* if sample_id_all */
  // };
  // Because of filename, the layout is not fixed.

  pid_t pid;
  ring_buffer->ReadValueAtOffset(&pid, sizeof(perf_event_header));
  return pid;
}

pid_t ReadSampleRecordPid(PerfEventRingBuffer* ring_buffer) {
  pid_t pid;
  ring_buffer->ReadValueAtOffset(
      &pid, offsetof(perf_event_stack_sample, sample_id.pid));
  return pid;
}

pid_t ReadUretprobesRecordPid(PerfEventRingBuffer* ring_buffer) {
  pid_t pid;
  ring_buffer->ReadValueAtOffset(
      &pid, offsetof(perf_event_empty_sample, sample_id.pid));
  return pid;
}

uint16_t ReadTracepointCommonType(PerfEventRingBuffer* ring_buffer) {
  uint16_t common_type = -1;
  ring_buffer->ReadValueAtOffset(
      &common_type, offsetof(perf_event_tracepoint_common, common_type));
  return common_type;
}

}  // namespace LinuxTracing
