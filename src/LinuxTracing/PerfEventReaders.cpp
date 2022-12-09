// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventReaders.h"

#include <linux/perf_event.h>
#include <stddef.h>

#include <bitset>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "KernelTracepoints.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "PerfEvent.h"
#include "PerfEventOpen.h"
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
                                                          perf_event_attr flags,
                                                          bool copy_stack_related_data = true) {
  ORBIT_CHECK(header.size >
              sizeof(perf_event_header) + sizeof(perf_event_sample_id_tid_time_streamid_cpu));

  PerfRecordSample event{};
  int current_offset = 0;

  ring_buffer->ReadRawAtOffset(&event.header, 0, sizeof(perf_event_header));
  current_offset += sizeof(perf_event_header);

  if ((flags.sample_type & PERF_SAMPLE_IDENTIFIER) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.sample_id, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_IP) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.ip, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_TID) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.pid, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
    ring_buffer->ReadRawAtOffset(&event.tid, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_TIME) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.time, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_ADDR) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.addr, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_ID) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.id, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_STREAM_ID) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.stream_id, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_CPU) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.cpu, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
    ring_buffer->ReadRawAtOffset(&event.res, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_PERIOD) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.period, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_CALLCHAIN) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.ips_size, current_offset, sizeof(uint64_t));

    current_offset += sizeof(uint64_t);
    if (copy_stack_related_data) {
      event.ips = make_unique_for_overwrite<uint64_t[]>(event.ips_size);
      ring_buffer->ReadRawAtOffset(event.ips.get(), current_offset,
                                   event.ips_size * sizeof(uint64_t));
    }
    current_offset += event.ips_size * sizeof(uint64_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_RAW) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.raw_size, current_offset, sizeof(uint32_t));
    current_offset += sizeof(uint32_t);
    event.raw_data = make_unique_for_overwrite<uint8_t[]>(event.raw_size);
    ring_buffer->ReadRawAtOffset(event.raw_data.get(), current_offset,
                                 event.raw_size * sizeof(uint8_t));
    current_offset += event.raw_size * sizeof(uint8_t);
  }

  if ((flags.sample_type & PERF_SAMPLE_REGS_USER) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.abi, current_offset, sizeof(uint64_t));

    current_offset += sizeof(uint64_t);
    if (event.abi != PERF_SAMPLE_REGS_ABI_NONE) {
      const int num_of_regs = std::bitset<64>(flags.sample_regs_user).count();
      if (copy_stack_related_data) {
        event.regs = make_unique_for_overwrite<uint64_t[]>(num_of_regs);
        ring_buffer->ReadRawAtOffset(event.regs.get(), current_offset,
                                     num_of_regs * sizeof(uint64_t));
      }
      current_offset += num_of_regs * sizeof(uint64_t);
    }
  }

  if ((flags.sample_type & PERF_SAMPLE_STACK_USER) != 0u) {
    ring_buffer->ReadRawAtOffset(&event.stack_size, current_offset, sizeof(uint64_t));
    current_offset += sizeof(uint64_t);
    if (event.stack_size != 0u && copy_stack_related_data) {
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
  // The flags here are in sync with uprobes_with_stack_and_sp_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from
  // uprobes_with_stack_and_sp_event_open
  const perf_event_attr flags{
      .sample_type =
          PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER | SAMPLE_TYPE_TID_TIME_STREAMID_CPU,
      .sample_regs_user = SAMPLE_REGS_USER_SP,
  };

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags);
  ring_buffer->SkipRecord(header);

  UprobesWithStackPerfEvent event{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .stream_id = res.stream_id,
              .pid = static_cast<pid_t>(res.pid),
              .tid = static_cast<pid_t>(res.tid),
              .regs = std::move(res.regs),
              .dyn_size = res.dyn_size,
              .data = std::move(res.stack_data),
          },
  };

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

PerfEvent ConsumeSchedWakeupWithOrWithoutCallchainPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                            const perf_event_header& header,
                                                            bool copy_stack_related_data) {
  // The flags here are in sync with tracepoint_with_callchain_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from
  // tracepoint_with_callchain_event_open
  const perf_event_attr flags{.sample_type = PERF_SAMPLE_CALLCHAIN | PERF_SAMPLE_RAW |
                                             SAMPLE_TYPE_TID_TIME_STREAMID_CPU |
                                             PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER,
                              .sample_regs_user = SAMPLE_REGS_USER_ALL};

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags, copy_stack_related_data);

  sched_wakeup_tracepoint_fixed sched_wakeup;
  std::memcpy(&sched_wakeup, res.raw_data.get(), sizeof(sched_wakeup_tracepoint_fixed));

  ring_buffer->SkipRecord(header);

  if (res.ips_size == 0 || !copy_stack_related_data) {
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
  return SchedWakeupWithCallchainPerfEvent{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              // The tracepoint format calls the woken tid "data.pid" but it's effectively the
              // thread id.
              .woken_tid = sched_wakeup.pid,
              .was_unblocked_by_tid = static_cast<pid_t>(res.tid),
              .was_unblocked_by_pid = static_cast<pid_t>(res.pid),
              .ips_size = res.ips_size,
              .ips = std::move(res.ips),
              .regs = std::move(res.regs),
              .data = std::move(res.stack_data),
          },
  };
}

PerfEvent ConsumeSchedWakeupWithOrWithoutStackPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                        const perf_event_header& header,
                                                        bool copy_stack_related_data) {
  // The flags here are in sync with tracepoint_with_stack_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from
  // tracepoint_with_stack_event_open
  const perf_event_attr flags{.sample_type = PERF_SAMPLE_RAW | SAMPLE_TYPE_TID_TIME_STREAMID_CPU |
                                             PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER,
                              .sample_regs_user = SAMPLE_REGS_USER_ALL};

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags, copy_stack_related_data);

  sched_wakeup_tracepoint_fixed sched_wakeup;
  std::memcpy(&sched_wakeup, res.raw_data.get(), sizeof(sched_wakeup_tracepoint_fixed));

  ring_buffer->SkipRecord(header);

  // If we did not receive the necessary data for a callstack, there is no need to return a
  // SchedWakeupWithStackPerfEvent.
  if (res.dyn_size == 0 || res.stack_data == nullptr || res.regs == nullptr ||
      !copy_stack_related_data) {
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
  return SchedWakeupWithStackPerfEvent{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              // See above for the usage of "pid" as a thread id.
              .woken_tid = sched_wakeup.pid,
              .was_unblocked_by_tid = static_cast<pid_t>(res.tid),
              .was_unblocked_by_pid = static_cast<pid_t>(res.pid),
              .regs = std::move(res.regs),
              .dyn_size = res.dyn_size,
              .data = std::move(res.stack_data),
          },
  };
}

PerfEvent ConsumeSchedSwitchWithOrWithoutStackPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                        const perf_event_header& header,
                                                        bool copy_stack_related_data) {
  // The flags here are in sync with tracepoint_with_stack_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from
  // tracepoint_with_stack_event_open
  const perf_event_attr flags{.sample_type = PERF_SAMPLE_RAW | SAMPLE_TYPE_TID_TIME_STREAMID_CPU |
                                             PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER,
                              .sample_regs_user = SAMPLE_REGS_USER_ALL};

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags, copy_stack_related_data);

  sched_switch_tracepoint sched_switch;
  std::memcpy(&sched_switch, res.raw_data.get(), sizeof(sched_switch_tracepoint));

  ring_buffer->SkipRecord(header);

  // If we did not receive the necessary data for a callstack, there is no need to return a
  // SchedSwitchWithStackPerfEvent.
  if (res.dyn_size == 0 || res.stack_data == nullptr || res.regs == nullptr ||
      !copy_stack_related_data) {
    return SchedSwitchPerfEvent{
        .timestamp = res.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
        .data =
            {
                .cpu = res.cpu,
                // As the tracepoint data does not include the pid of the process that the thread
                // being switched out belongs to, we use the pid set by perf_event_open in the
                // corresponding generic field of the PERF_RECORD_SAMPLE.
                // Note, though, that this value is -1 when the switch out is caused by the thread
                // exiting. This is not the case for data.prev_pid, whose value is always correct
                // as it comes directly from the tracepoint data.
                .prev_pid_or_minus_one = static_cast<pid_t>(res.pid),
                .prev_tid = sched_switch.prev_pid,
                .prev_state = sched_switch.prev_state,
                .next_tid = sched_switch.next_pid,
            },
    };
  }
  return SchedSwitchWithStackPerfEvent{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .cpu = res.cpu,
              // See above why we use "res.pid" as process id of the previous thread.
              .prev_pid_or_minus_one = static_cast<pid_t>(res.pid),
              .prev_tid = sched_switch.prev_pid,
              .prev_state = sched_switch.prev_state,
              .next_tid = sched_switch.next_pid,
              .regs = std::move(res.regs),
              .dyn_size = res.dyn_size,
              .data = std::move(res.stack_data),
          },
  };
}

PerfEvent ConsumeSchedSwitchWithOrWithoutCallchainPerfEvent(PerfEventRingBuffer* ring_buffer,
                                                            const perf_event_header& header,
                                                            bool copy_stack_related_data) {
  // The flags here are in sync with tracepoint_with_callchain_event_open in PerfEventOpen.
  // TODO(b/242020362): use the same perf_event_attr object from
  // tracepoint_with_callchain_event_open
  const perf_event_attr flags{.sample_type = PERF_SAMPLE_CALLCHAIN | PERF_SAMPLE_RAW |
                                             SAMPLE_TYPE_TID_TIME_STREAMID_CPU |
                                             PERF_SAMPLE_REGS_USER | PERF_SAMPLE_STACK_USER,
                              .sample_regs_user = SAMPLE_REGS_USER_ALL};

  PerfRecordSample res = ConsumeRecordSample(ring_buffer, header, flags, copy_stack_related_data);

  sched_switch_tracepoint sched_switch;
  std::memcpy(&sched_switch, res.raw_data.get(), sizeof(sched_switch_tracepoint));

  ring_buffer->SkipRecord(header);

  if (res.ips_size == 0 || !copy_stack_related_data) {
    return SchedSwitchPerfEvent{
        .timestamp = res.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
        .data =
            {
                .cpu = res.cpu,
                // See ConsumeSchedSwitchWithOrWithoutStackPerfEvent for why the pid can be -1.
                .prev_pid_or_minus_one = static_cast<pid_t>(res.pid),
                .prev_tid = sched_switch.prev_pid,
                .prev_state = sched_switch.prev_state,
                .next_tid = sched_switch.next_pid,
            },
    };
  }

  return SchedSwitchWithCallchainPerfEvent{
      .timestamp = res.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .cpu = res.cpu,
              // See ConsumeSchedSwitchWithOrWithoutStackPerfEvent for why the pid can be -1.
              .prev_pid_or_minus_one = static_cast<pid_t>(res.pid),
              .prev_tid = sched_switch.prev_pid,
              .prev_state = sched_switch.prev_state,
              .next_tid = sched_switch.next_pid,
              .ips_size = res.ips_size,
              .ips = std::move(res.ips),
              .regs = std::move(res.regs),
              .data = std::move(res.stack_data),
          },
  };
}

template <typename EventType, typename StructType>
[[nodiscard]] EventType ConsumeGpuEvent(PerfEventRingBuffer* ring_buffer,
                                        const perf_event_header& header) {
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
  const auto data_loc_size = static_cast<int16_t>(typed_tracepoint_data.timeline >> 16);
  const auto data_loc_offset = static_cast<int16_t>(typed_tracepoint_data.timeline & 0x00ff);
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
