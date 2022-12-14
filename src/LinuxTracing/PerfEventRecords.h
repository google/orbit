// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_RECORDS_H_
#define LINUX_TRACING_PERF_EVENT_RECORDS_H_

#include <cstdint>

#include "PerfEventOpen.h"

namespace orbit_linux_tracing {

// This struct must be in sync with the SAMPLE_TYPE_TID_TIME_STREAMID_CPU in PerfEventOpen.h, as the
// bits set in perf_event_attr::sample_type determine the fields this struct should have.
struct __attribute__((__packed__)) PerfEventSampleIdTidTimeStreamidCpu {
  uint32_t pid, tid;  /* if PERF_SAMPLE_TID */
  uint64_t time;      /* if PERF_SAMPLE_TIME */
  uint64_t stream_id; /* if PERF_SAMPLE_STREAM_ID */
  uint32_t cpu, res;  /* if PERF_SAMPLE_CPU */
};

struct __attribute__((__packed__)) PerfEventForkExit {
  perf_event_header header;
  uint32_t pid, ppid;
  uint32_t tid, ptid;
  uint64_t time;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
};

// This struct must be in sync with the SAMPLE_REGS_USER_ALL in PerfEventOpen.h.
struct __attribute__((__packed__)) PerfEventSampleRegsUserAll {
  uint64_t ax;
  uint64_t bx;
  uint64_t cx;
  uint64_t dx;
  uint64_t si;
  uint64_t di;
  uint64_t bp;
  uint64_t sp;
  uint64_t ip;
  uint64_t flags;
  uint64_t cs;
  uint64_t ss;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
};

// This struct must be in sync with the SAMPLE_REGS_USER_AX in PerfEventOpen.h.
struct __attribute__((__packed__)) PerfEventSampleRegsUserAx {
  uint64_t abi;
  uint64_t ax;
};

// This struct must be in sync with the SAMPLE_REGS_USER_SP_IP in PerfEventOpen.h.
struct __attribute__((__packed__)) PerfEventSampleRegsUserSpIp {
  uint64_t abi;
  uint64_t sp;
  uint64_t ip;
};

// This struct must be in sync with the SAMPLE_REGS_USER_SP in PerfEventOpen.h.
struct __attribute__((__packed__)) PerfEventSampleRegsUserSp {
  uint64_t sp;
};

// This struct must be in sync with the SAMPLE_REGS_USER_SP_IP_ARGUMENTS in PerfEventOpen.h.
struct __attribute__((__packed__)) PerfEventSampleRegsUserSpIpArguments {
  uint64_t abi;
  uint64_t cx;
  uint64_t dx;
  uint64_t si;
  uint64_t di;
  uint64_t sp;
  uint64_t ip;
  uint64_t r8;
  uint64_t r9;
};

struct __attribute__((__packed__)) PerfEventSampleStackUser8bytes {
  uint64_t size;
  uint64_t top8bytes;
  uint64_t dyn_size;
};

struct __attribute__((__packed__)) PerfEventStackSampleFixed {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  uint64_t abi;
  PerfEventSampleRegsUserAll regs;
  // Following this field there are the following fields, which we read dynamically:
  // uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  // char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  // uint64_t dyn_size;                 /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

struct __attribute__((__packed__)) PerfEventSpIpArguments8bytesSample {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  PerfEventSampleRegsUserSpIpArguments regs;
  PerfEventSampleStackUser8bytes stack;
};

struct __attribute__((__packed__)) PerfEventSpIp8bytesSample {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  PerfEventSampleRegsUserSpIp regs;
  PerfEventSampleStackUser8bytes stack;
};

struct __attribute__((__packed__)) PerfEventSpStackUserSampleFixed {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  uint64_t abi;
  PerfEventSampleRegsUserSp regs;
  // Following this field there are the following fields, which we read dynamically:
  // uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  // char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  // uint64_t dyn_size;                 /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

struct __attribute__((__packed__)) PerfEventEmptySample {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
};

struct __attribute__((__packed__)) PerfEventAxSample {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  PerfEventSampleRegsUserAx regs;
};

template <typename TracepointT>
struct __attribute__((__packed__)) PerfEventRawSample {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  uint32_t size;
  TracepointT data;
};

struct __attribute__((__packed__)) PerfEventRawSampleFixed {
  perf_event_header header;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
  uint32_t size;
  // The rest of the sample is a char[size] that we read dynamically.
};

struct __attribute__((__packed__)) PerfEventMmapUpToPgoff {
  perf_event_header header;
  uint32_t pid;
  uint32_t tid;
  uint64_t address;
  uint64_t length;
  uint64_t page_offset;
  // OMITTED: char filename[]
  // OMITTED: perf_event_sample_id_tid_time_streamid_cpu sample_id;
};

struct __attribute__((__packed__)) PerfEventLost {
  perf_event_header header;
  uint64_t id;
  uint64_t lost;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
};

struct __attribute__((__packed__)) PerfEventThrottleUnthrottle {
  perf_event_header header;
  uint64_t time;
  uint64_t id;
  uint64_t lost;
  PerfEventSampleIdTidTimeStreamidCpu sample_id;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_RECORDS_H_
