// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventReaders.h"

#include <linux/perf_event.h>
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
#include "PerfEvent.h"
#include "PerfEventOrderedStream.h"
#include "PerfEventRecords.h"
#include "PerfEventRingBuffer.h"

namespace orbit_linux_tracing {

// This struct is supposed to resemble the perf_record_sample, all commented out
// fields are fields we don't currently use anywhere. This is only used to communicate
// between ConsumeRecordSample and the rest of the consumer functions
struct PerfRecordSample {
  perf_event_header header;

  uint64_t sample_id; /* if PERF_SAMPLE_IDENTIFIER */
  uint64_t ip;        /* if PERF_SAMPLE_IP */
  uint32_t pid, tid;  /* if PERF_SAMPLE_TID */
  uint64_t time;      /* if PERF_SAMPLE_TIME */
  uint64_t addr;      /* if PERF_SAMPLE_ADDR */
  uint64_t id;        /* if PERF_SAMPLE_ID */
  uint64_t stream_id; /* if PERF_SAMPLE_STREAM_ID */
  uint32_t cpu, res;  /* if PERF_SAMPLE_CPU */
  uint64_t period;    /* if PERF_SAMPLE_PERIOD */

  // struct read_format v;                 /* if PERF_SAMPLE_READ */

  uint64_t ips_size;               /* if PERF_SAMPLE_CALLCHAIN */
  std::unique_ptr<uint64_t[]> ips; /* if PERF_SAMPLE_CALLCHAIN */

  uint32_t raw_size;                   /* if PERF_SAMPLE_RAW */
  std::unique_ptr<uint8_t[]> raw_data; /* if PERF_SAMPLE_RAW */

  // uint64_t bnr;                        /* if PERF_SAMPLE_BRANCH_STACK */
  // struct perf_branch_entry lbr[bnr];   /* if PERF_SAMPLE_BRANCH_STACK */

  uint64_t abi;                     /* if PERF_SAMPLE_REGS_USER */
  std::unique_ptr<uint64_t[]> regs; /* if PERF_SAMPLE_REGS_USER */

  uint64_t stack_size;                   /* if PERF_SAMPLE_STACK_USER */
  std::unique_ptr<uint8_t[]> stack_data; /* if PERF_SAMPLE_STACK_USER */
  uint64_t dyn_size;                     /* if PERF_SAMPLE_STACK_USER && size != 0 */

  // uint64_t weight;                     /* if PERF_SAMPLE_WEIGHT */
  // uint64_t data_src;                   /* if PERF_SAMPLE_DATA_SRC */
  // uint64_t transaction;                /* if PERF_SAMPLE_TRANSACTION */
  // uint64_t abi;                        /* if PERF_SAMPLE_REGS_INTR */
  // uint64_t regs[weight(mask)];         /* if PERF_SAMPLE_REGS_INTR */
  // uint64_t phys_addr;                  /* if PERF_SAMPLE_PHYS_ADDR */
  // uint64_t cgroup;                     /* if PERF_SAMPLE_CGROUP */
};

[[nodiscard]] static PerfRecordSample ConsumeRecordSample(PerfEventRingBuffer* ring_buffer,
                                                          const perf_event_header& header,
                                                          perf_event_attr flags) {
  ORBIT_CHECK(header.size >
              sizeof(perf_event_header) + sizeof(perf_event_sample_id_tid_time_streamid_cpu));

  PerfRecordSample event{};
  int current_offset = 0;

  ring_buffer->ReadRawAtOffset(&event.header, 0, sizeof(perf_event_header));
  current_offset += sizeof(perf_event_header);

  if (flags.sample_type & PERF_SAMPLE_IDENTIFIER) {
    ring_buffer->ReadRawAtOffset(&event.sample_id, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_IP) {
    ring_buffer->ReadRawAtOffset(&event.ip, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_TID) {
    ring_buffer->ReadRawAtOffset(&event.pid, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
    ring_buffer->ReadRawAtOffset(&event.tid, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
  }

  if (flags.sample_type & PERF_SAMPLE_TIME) {
    ring_buffer->ReadRawAtOffset(&event.time, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_ADDR) {
    ring_buffer->ReadRawAtOffset(&event.addr, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_ID) {
    ring_buffer->ReadRawAtOffset(&event.id, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_STREAM_ID) {
    ring_buffer->ReadRawAtOffset(&event.stream_id, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_CPU) {
    ring_buffer->ReadRawAtOffset(&event.cpu, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
    ring_buffer->ReadRawAtOffset(&event.res, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
  }

  if (flags.sample_type & PERF_SAMPLE_PERIOD) {
    ring_buffer->ReadRawAtOffset(&event.period, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_CALLCHAIN) {
    ring_buffer->ReadRawAtOffset(&event.ips_size, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
    event.ips = make_unique_for_overwrite<uint64_t[]>(event.ips_size);
    ring_buffer->ReadRawAtOffset(event.ips.get(), current_offset,
                                 event.ips_size * sizeof(uint64_t));
    current_offset += event.ips_size * sizeof(uint64_t);
  }

  if (flags.sample_type & PERF_SAMPLE_RAW) {
    ring_buffer->ReadRawAtOffset(&event.raw_size, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
    event.raw_data = make_unique_for_overwrite<uint8_t[]>(event.raw_size);
    ring_buffer->ReadRawAtOffset(event.raw_data.get(), current_offset,
                                 event.raw_size * sizeof(uint8_t));
    current_offset += event.raw_size * sizeof(uint8_t);
  }

  if (flags.sample_type & PERF_SAMPLE_REGS_USER) {
    ring_buffer->ReadRawAtOffset(&event.abi, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
    if (event.abi != PERF_SAMPLE_REGS_ABI_NONE) {
      const int num_of_regs = std::bitset<64>(flags.sample_regs_user).count();
      event.regs = make_unique_for_overwrite<uint64_t[]>(num_of_regs);
      ring_buffer->ReadRawAtOffset(event.regs.get(), current_offset,
                                   num_of_regs * sizeof(uint64_t));
      current_offset += num_of_regs * sizeof(uint64_t);
    }
  }

  if (flags.sample_type & PERF_SAMPLE_STACK_USER) {
    ring_buffer->ReadRawAtOffset(&event.stack_size, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
    if (event.stack_size != 0u) {
      // dyn_size comes after the actual stack but we read it first so
      // we can use it to not copy unnessary parts of the stack.
      ring_buffer->ReadRawAtOffset(
          &event.dyn_size, current_offset + (event.stack_size * sizeof(uint8_t)), sizeof(uint64_t));
      event.stack_data = make_unique_for_overwrite<uint8_t[]>(event.dyn_size);
      ring_buffer->ReadRawAtOffset(event.stack_data.get(), current_offset,
                                   event.dyn_size * sizeof(uint8_t));
    }
    current_offset += event.stack_size * sizeof(uint8_t);
    if (event.stack_size != 0u) {
      // dyn_size was already read but its offset wasn't increased.
      // we increase it here.
      current_offset += sizeof(uint64_t);
    }
  }

  return event;
}

void ReadPerfSampleIdAll(PerfEventRingBuffer* ring_buffer, const perf_event_header& header,
                         perf_event_sample_id_tid_time_streamid_cpu* sample_id) {
  ORBIT_CHECK(sample_id != nullptr);
  ORBIT_CHECK(header.size >
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
  ORBIT_CHECK(header.size > (filename_offset + sizeof(perf_event_sample_id_tid_time_streamid_cpu)));
  size_t filename_size =
      header.size - filename_offset - sizeof(perf_event_sample_id_tid_time_streamid_cpu);
  std::vector<char> filename_vector(filename_size);
  ring_buffer->ReadRawAtOffset(&filename_vector[0], filename_offset, filename_size);
  // This is a bit paranoid but you never know
  filename_vector.back() = '\0';
  std::string filename(filename_vector.data());

  ring_buffer->SkipRecord(header);

  // Workaround for gcc's "cannot bind packed field ... to ‘long unsigned int&’"
  uint64_t timestamp = sample_id.time;
  int32_t pid = static_cast<int32_t>(sample_id.pid);

  const bool executable = (header.misc & PERF_RECORD_MISC_MMAP_DATA) == 0;

  // mmap events for anonymous maps have filename "//anon". Make it "" for simplicity.
  if (filename == "//anon") {
    filename.clear();
  }
  // mmap events for anonymous maps usually have page_offset == address. Make it 0 for clarity.
  uint64_t page_offset = mmap_event.page_offset;
  if ((filename.empty() || filename[0] == '[') && page_offset == mmap_event.address) {
    page_offset = 0;
  }

  // Consider moving this to MMAP2 event which has more information (like flags)
  return MmapPerfEvent{
      .timestamp = timestamp,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .address = mmap_event.address,
              .length = mmap_event.length,
              .page_offset = page_offset,
              .filename = std::move(filename),
              .executable = executable,
              .pid = pid,
          },
  };
}

StackSamplePerfEvent ConsumeStackSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                                 const perf_event_header& header) {
  // The flags here are in sync with stack_sample_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from stack_sample_event_open
  const perf_event_attr flags{
      .sample_type =
          PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER | SAMPLE_TYPE_TID_TIME_STREAMID_CPU,
      .sample_regs_user = SAMPLE_REGS_USER_ALL,
  };

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags);

  StackSamplePerfEvent event{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(res.pid),
              .tid = static_cast<pid_t>(res.tid),
              .regs = std::move(res.regs),
              .dyn_size = res.dyn_size,
              .data = std::move(res.stack_data),
          },
  };

  ring_buffer->SkipRecord(header);
  return event;
}

CallchainSamplePerfEvent ConsumeCallchainSamplePerfEvent(PerfEventRingBuffer* ring_buffer,
                                                         const perf_event_header& header) {
  // The flags here are in sync with callchain_sample_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from callchain_sample_event_open
  const perf_event_attr flags{
      .sample_type = PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER | PERF_SAMPLE_CALLCHAIN |
                     SAMPLE_TYPE_TID_TIME_STREAMID_CPU,
      .sample_regs_user = SAMPLE_REGS_USER_ALL,
  };

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags);

  CallchainSamplePerfEvent event{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(res.pid),
              .tid = static_cast<pid_t>(res.tid),
              .ips_size = res.ips_size,
              .ips = std::move(res.ips),
              .regs = std::move(res.regs),
              .data = std::move(res.stack_data),
          },
  };

  ring_buffer->SkipRecord(header);
  return event;
}

UprobesWithStackPerfEvent ConsumeUprobeWithStackPerfEvent(PerfEventRingBuffer* ring_buffer,
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

  size_t offset_of_size = offsetof(perf_event_sp_stack_user_sample_fixed, regs) +
                          sizeof(perf_event_sample_regs_user_sp);
  size_t offset_of_data = offset_of_size + sizeof(uint64_t);

  uint64_t size = 0;
  ring_buffer->ReadValueAtOffset(&size, offset_of_size);

  size_t offset_of_dyn_size = offset_of_data + (size * sizeof(char));

  uint64_t dyn_size = 0;
  ring_buffer->ReadValueAtOffset(&dyn_size, offset_of_dyn_size);

  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  ring_buffer->ReadValueAtOffset(&sample_id,
                                 offsetof(perf_event_sp_stack_user_sample_fixed, sample_id));

  perf_event_sample_regs_user_sp regs;
  ring_buffer->ReadValueAtOffset(&regs, offsetof(perf_event_sp_stack_user_sample_fixed, regs));

  UprobesWithStackPerfEvent event{
      .timestamp = sample_id.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .stream_id = sample_id.stream_id,
              .pid = static_cast<pid_t>(sample_id.pid),
              .tid = static_cast<pid_t>(sample_id.tid),
              .regs = regs,
              .dyn_size = dyn_size,
              .data = make_unique_for_overwrite<uint8_t[]>(dyn_size),
          },
  };
  ring_buffer->ReadRawAtOffset(event.data.data.get(), offset_of_data, dyn_size);
  ring_buffer->SkipRecord(header);
  return event;
}

GenericTracepointPerfEvent ConsumeGenericTracepointPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                             const perf_event_header& header) {
  // The flags here are in sync with generic_event_attr in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from generic_event_attr
  const perf_event_attr flags{
      .sample_type = SAMPLE_TYPE_TID_TIME_STREAMID_CPU,
  };

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags);

  GenericTracepointPerfEvent event{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(res.pid),
              .tid = static_cast<pid_t>(res.tid),
              .cpu = res.cpu,
          },
  };

  ring_buffer->SkipRecord(header);
  return event;
}

SchedWakeupPerfEvent ConsumeSchedWakeupPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                 const perf_event_header& header) {
  // The flags here are in sync with tracepoint_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from tracepoint_event_open
  const perf_event_attr flags{
      .sample_type = PERF_SAMPLE_RAW | SAMPLE_TYPE_TID_TIME_STREAMID_CPU,
  };

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags);

  sched_wakeup_tracepoint_fixed sched_wakeup;
  std::memcpy(&sched_wakeup, res.raw_data.get(), sizeof(sched_wakeup_tracepoint_fixed));

  ring_buffer->SkipRecord(header);
  return SchedWakeupPerfEvent{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              // The tracepoint format calls the woken tid "data.pid" but it's effectively the
              // thread id.
              .woken_tid = sched_wakeup.pid,
              .was_unblocked_by_tid = static_cast<pid_t>(res.tid),
              .was_unblocked_by_pid = static_cast<pid_t>(res.pid),
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
