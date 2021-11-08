// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventReaders.h"

#include <stddef.h>
#include <string.h>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "KernelTracepoints.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/ThreadUtils.h"
#include "PerfEventOrderedStream.h"
#include "PerfEventRecords.h"
#include "PerfEventRingBuffer.h"

namespace orbit_linux_tracing {

void ReadPerfSampleIdAll(PerfEventRingBuffer* ring_buffer, const perf_event_header& header,
                         perf_event_sample_id_tid_time_streamid_cpu* sample_id) {
  CHECK(sample_id != nullptr);
  CHECK(header.size >
        sizeof(perf_event_header) + sizeof(perf_event_sample_id_tid_time_streamid_cpu));
  // sample_id_all is always the last field in the event
  uint64_t offset = header.size - sizeof(perf_event_sample_id_tid_time_streamid_cpu);
  ring_buffer->ReadValueAtOffset(sample_id, offset);
}

uint64_t ReadSampleRecordTime(PerfEventRingBuffer* ring_buffer) {
  uint64_t time;
  // All PERF_RECORD_SAMPLEs start with
  //   perf_event_header header;
  //   perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ring_buffer->ReadValueAtOffset(
      &time,
      sizeof(perf_event_header) + offsetof(perf_event_sample_id_tid_time_streamid_cpu, time));
  return time;
}

uint64_t ReadSampleRecordStreamId(PerfEventRingBuffer* ring_buffer) {
  uint64_t stream_id;
  // All PERF_RECORD_SAMPLEs start with
  //   perf_event_header header;
  //   perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ring_buffer->ReadValueAtOffset(
      &stream_id,
      sizeof(perf_event_header) + offsetof(perf_event_sample_id_tid_time_streamid_cpu, stream_id));
  return stream_id;
}

pid_t ReadSampleRecordPid(PerfEventRingBuffer* ring_buffer) {
  pid_t pid;
  // All PERF_RECORD_SAMPLEs start with
  //   perf_event_header header;
  //   perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ring_buffer->ReadValueAtOffset(
      &pid, sizeof(perf_event_header) + offsetof(perf_event_sample_id_tid_time_streamid_cpu, pid));
  return pid;
}

uint64_t ReadThrottleUnthrottleRecordTime(PerfEventRingBuffer* ring_buffer) {
  // Note that perf_event_throttle_unthrottle::time and
  // perf_event_sample_id_tid_time_streamid_cpu::time differ a bit. Use the latter as we use that
  // for all other events.
  uint64_t time;
  ring_buffer->ReadValueAtOffset(&time,
                                 offsetof(perf_event_throttle_unthrottle, sample_id) +
                                     offsetof(perf_event_sample_id_tid_time_streamid_cpu, time));
  return time;
}

MmapPerfEvent ConsumeMmapPerfEvent(PerfEventRingBuffer* ring_buffer,
                                   const perf_event_header& header) {
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

  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ReadPerfSampleIdAll(ring_buffer, header, &sample_id);

  perf_event_mmap_up_to_pgoff mmap_event;
  ring_buffer->ReadValueAtOffset(&mmap_event, 0);

  // read filename
  size_t filename_offset = sizeof(perf_event_mmap_up_to_pgoff);
  // strictly > because filename is null-terminated string
  CHECK(header.size > (filename_offset + sizeof(perf_event_sample_id_tid_time_streamid_cpu)));
  size_t filename_size =
      header.size - filename_offset - sizeof(perf_event_sample_id_tid_time_streamid_cpu);
  std::vector<char> filename_vector(filename_size);
  ring_buffer->ReadRawAtOffset(&filename_vector[0], filename_offset, filename_size);
  // This is a bit paranoid but you never know
  filename_vector[filename_size - 1] = '\0';
  std::string filename(filename_vector.data());

  ring_buffer->SkipRecord(header);

  // Workaround for gcc's "cannot bind packed field ... to ‘long unsigned int&’"
  uint64_t timestamp = sample_id.time;
  int32_t pid = static_cast<int32_t>(sample_id.pid);

  // Consider moving this to MMAP2 event which has more information (like flags)
  return MmapPerfEvent{
      .timestamp = timestamp,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .address = mmap_event.address,
              .length = mmap_event.length,
              .page_offset = mmap_event.page_offset,
              .filename = std::move(filename),
              .pid = pid,
          },
  };
}

StackSamplePerfEvent ConsumeStackSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                                 const perf_event_header& header) {
  // We expect the following layout of the perf event:
  //  struct {
  //    struct perf_event_header header;
  //    u64 sample_id;          /* if PERF_SAMPLE_IDENTIFIER */
  //    u32 pid, tid;           /* if PERF_SAMPLE_TID */
  //    u64 time;               /* if PERF_SAMPLE_TIME */
  //    u64 stream_id;          /* if PERF_SAMPLE_STREAM_ID */
  //    u32 cpu, res;           /* if PERF_SAMPLE_CPU */
  //    u64 abi;                /* if PERF_SAMPLE_REGS_USER */
  //    u64 regs[weight(mask)]; /* if PERF_SAMPLE_REGS_USER */
  //    u64 size;               /* if PERF_SAMPLE_STACK_USER */
  //    char data[size];        /* if PERF_SAMPLE_STACK_USER */
  //    u64 dyn_size;           /* if PERF_SAMPLE_STACK_USER && size != 0 */
  //  };
  // Unfortunately, the value of `size` is not constant, so we need to compute the offsets by hand,
  // rather than relying on a struct.

  size_t offset_of_size =
      offsetof(perf_event_stack_sample_fixed, regs) + sizeof(perf_event_sample_regs_user_all);
  size_t offset_of_data = offset_of_size + sizeof(uint64_t);

  uint64_t size = 0;
  ring_buffer->ReadValueAtOffset(&size, offset_of_size);

  size_t offset_of_dyn_size = offset_of_data + (size * sizeof(char));

  uint64_t dyn_size = 0;
  ring_buffer->ReadValueAtOffset(&dyn_size, offset_of_dyn_size);

  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ring_buffer->ReadValueAtOffset(&sample_id, offsetof(perf_event_stack_sample_fixed, sample_id));

  StackSamplePerfEvent event{
      .timestamp = sample_id.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(sample_id.pid),
              .tid = static_cast<pid_t>(sample_id.tid),
              .regs = make_unique_for_overwrite<perf_event_sample_regs_user_all>(),
              .dyn_size = dyn_size,
              .data = make_unique_for_overwrite<char[]>(dyn_size),
          },
  };

  ring_buffer->ReadValueAtOffset(event.data.regs.get(),
                                 offsetof(perf_event_stack_sample_fixed, regs));
  ring_buffer->ReadRawAtOffset(event.data.data.get(), offset_of_data, dyn_size);
  ring_buffer->SkipRecord(header);
  return event;
}

CallchainSamplePerfEvent ConsumeCallchainSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                                         const perf_event_header& header) {
  // We expect the following layout of the perf event:
  //  struct {
  //    struct perf_event_header header;
  //    u64 sample_id;          /* if PERF_SAMPLE_IDENTIFIER */
  //    u32 pid, tid;           /* if PERF_SAMPLE_TID */
  //    u64 time;               /* if PERF_SAMPLE_TIME */
  //    u64 stream_id;          /* if PERF_SAMPLE_STREAM_ID */
  //    u32 cpu, res;           /* if PERF_SAMPLE_CPU */
  //    u64 nr;                 /* if PERF_SAMPLE_CALLCHAIN */
  //    u64 ips[nr];            /* if PERF_SAMPLE_CALLCHAIN */
  //    u64 abi;                /* if PERF_SAMPLE_REGS_USER */
  //    u64 regs[weight(mask)]; /* if PERF_SAMPLE_REGS_USER */
  //    u64 size;               /* if PERF_SAMPLE_STACK_USER */
  //    char data[size];        /* if PERF_SAMPLE_STACK_USER */
  //    u64 dyn_size;           /* if PERF_SAMPLE_STACK_USER && size != 0 */
  //  };
  // Unfortunately, the number of `ips` is dynamic, so we need to compute the offsets by hand,
  // rather than relying on a struct.
  uint64_t nr = 0;
  ring_buffer->ReadValueAtOffset(&nr, offsetof(perf_event_callchain_sample_fixed, nr));

  const uint64_t size_of_ips_in_bytes = nr * sizeof(uint64_t);

  const size_t offset_of_ips = offsetof(perf_event_callchain_sample_fixed, nr) +
                               sizeof(perf_event_callchain_sample_fixed::nr);
  const size_t offset_of_regs_user_struct = offset_of_ips + size_of_ips_in_bytes;
  // Note that perf_event_sample_regs_user_all contains abi and the regs array.
  const size_t offset_of_size =
      offset_of_regs_user_struct + sizeof(perf_event_sample_regs_user_all);
  const size_t offset_of_data = offset_of_size + sizeof(uint64_t);

  uint64_t size = 0;
  ring_buffer->ReadRawAtOffset(&size, offset_of_size, sizeof(uint64_t));

  const size_t offset_of_dyn_size = offset_of_data + size;
  uint64_t dyn_size = 0;
  ring_buffer->ReadRawAtOffset(&dyn_size, offset_of_dyn_size, sizeof(uint64_t));
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ring_buffer->ReadValueAtOffset(&sample_id,
                                 offsetof(perf_event_callchain_sample_fixed, sample_id));

  CallchainSamplePerfEvent event{
      .timestamp = sample_id.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(sample_id.pid),
              .tid = static_cast<pid_t>(sample_id.tid),
              .ips_size = nr,
              .ips = make_unique_for_overwrite<uint64_t[]>(nr),
              .regs = make_unique_for_overwrite<perf_event_sample_regs_user_all>(),
              .data = make_unique_for_overwrite<char[]>(dyn_size),
          },
  };

  ring_buffer->ReadRawAtOffset(event.data.ips.get(), offset_of_ips, size_of_ips_in_bytes);
  ring_buffer->ReadRawAtOffset(event.data.regs.get(), offset_of_regs_user_struct,
                               sizeof(perf_event_sample_regs_user_all));
  ring_buffer->ReadRawAtOffset(event.data.data.get(), offset_of_data, dyn_size);

  ring_buffer->SkipRecord(header);
  return event;
}

GenericTracepointPerfEvent ConsumeGenericTracepointPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                             const perf_event_header& header) {
  perf_event_raw_sample_fixed ring_buffer_record;
  ring_buffer->ReadRawAtOffset(&ring_buffer_record, 0, sizeof(perf_event_raw_sample_fixed));
  GenericTracepointPerfEvent event{
      .timestamp = ring_buffer_record.sample_id.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
              .tid = static_cast<pid_t>(ring_buffer_record.sample_id.tid),
              .cpu = ring_buffer_record.sample_id.cpu,
          },
  };

  ring_buffer->SkipRecord(header);
  return event;
}

SchedWakeupPerfEvent ConsumeSchedWakeupPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                 const perf_event_header& header) {
  CHECK(header.size >= sizeof(perf_event_raw_sample_fixed));
  perf_event_raw_sample_fixed ring_buffer_record;
  ring_buffer->ReadRawAtOffset(&ring_buffer_record, 0, sizeof(perf_event_raw_sample_fixed));

  // The last fields of the sched:sched_wakeup tracepoint aren't always the same, depending on the
  // kernel version. Fortunately we only need the first fields, which are always the same, so only
  // read those. See `sched_wakeup_tracepoint_fixed`.
  CHECK(ring_buffer_record.size >= sizeof(sched_wakeup_tracepoint_fixed));
  sched_wakeup_tracepoint_fixed sched_wakeup;
  ring_buffer->ReadRawAtOffset(
      &sched_wakeup,
      offsetof(perf_event_raw_sample_fixed, size) + sizeof(perf_event_raw_sample_fixed::size),
      sizeof(sched_wakeup_tracepoint_fixed));

  ring_buffer->SkipRecord(header);
  return SchedWakeupPerfEvent{
      .timestamp = ring_buffer_record.sample_id.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              // The tracepoint format calls the woken tid "data.pid" but it's effectively the
              // thread id.
              .woken_tid = sched_wakeup.pid,
          },
  };
}

template <typename EventType, typename StructType>
EventType ConsumeGpuEvent(PerfEventRingBuffer* ring_buffer, const perf_event_header& header) {
  uint32_t tracepoint_size;
  ring_buffer->ReadValueAtOffset(&tracepoint_size, offsetof(perf_event_raw_sample_fixed, size));

  perf_event_raw_sample_fixed ring_buffer_record;
  ring_buffer->ReadRawAtOffset(&ring_buffer_record, 0, sizeof(perf_event_raw_sample_fixed));

  std::unique_ptr<uint8_t[]> tracepoint_data =
      make_unique_for_overwrite<uint8_t[]>(tracepoint_size);
  ring_buffer->ReadRawAtOffset(
      tracepoint_data.get(),
      offsetof(perf_event_raw_sample_fixed, size) + sizeof(perf_event_raw_sample_fixed::size),
      tracepoint_size);
  const StructType& typed_tracepoint_data =
      *reinterpret_cast<const StructType*>(tracepoint_data.get());
  const int16_t data_loc_size = static_cast<int16_t>(typed_tracepoint_data.timeline >> 16);
  const int16_t data_loc_offset = static_cast<int16_t>(typed_tracepoint_data.timeline & 0x00ff);
  std::vector<char> data_loc_data(data_loc_size);
  std::memcpy(&data_loc_data[0],
              reinterpret_cast<const char*>(tracepoint_data.get()) + data_loc_offset,
              data_loc_size);
  data_loc_data[data_loc_data.size() - 1] = 0;

  // dma_fence_signaled events can be out of order of timestamp even on the same ring buffer, hence
  // why PerfEventOrderedStream::kNone. To be safe, do the same for the other GPU events.
  EventType event{
      .timestamp = ring_buffer_record.sample_id.time,
      .ordered_stream = PerfEventOrderedStream::kNone,
      .data =
          {
              .pid = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
              .tid = static_cast<pid_t>(ring_buffer_record.sample_id.tid),
              .context = typed_tracepoint_data.context,
              .seqno = typed_tracepoint_data.seqno,
              .timeline_string = std::string(&data_loc_data[0]),
          },
  };

  ring_buffer->SkipRecord(header);
  return event;
}

AmdgpuCsIoctlPerfEvent ConsumeAmdgpuCsIoctlPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                     const perf_event_header& header) {
  return ConsumeGpuEvent<AmdgpuCsIoctlPerfEvent, amdgpu_cs_ioctl_tracepoint>(ring_buffer, header);
}

AmdgpuSchedRunJobPerfEvent ConsumeAmdgpuSchedRunJobPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                             const perf_event_header& header) {
  return ConsumeGpuEvent<AmdgpuSchedRunJobPerfEvent, amdgpu_sched_run_job_tracepoint>(ring_buffer,
                                                                                      header);
}

DmaFenceSignaledPerfEvent ConsumeDmaFenceSignaledPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                           const perf_event_header& header) {
  return ConsumeGpuEvent<DmaFenceSignaledPerfEvent, dma_fence_signaled_tracepoint>(ring_buffer,
                                                                                   header);
}

}  // namespace orbit_linux_tracing
