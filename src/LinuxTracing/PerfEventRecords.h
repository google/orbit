// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_RECORDS_H_
#define LINUX_TRACING_PERF_EVENT_RECORDS_H_

#include <cstdint>

#include "PerfEventOpen.h"

namespace orbit_linux_tracing {

// This struct must be in sync with the `kSampleTypeTidTimeStreamidCpu` in PerfEventOpen.h, as the
// bits set in perf_event_attr::sample_type determine the fields this struct should have.
struct __attribute__((__packed__)) RingBufferSampleIdTidTimeStreamidCpu {
  uint32_t pid, tid;  /* if PERF_SAMPLE_TID */
  uint64_t time;      /* if PERF_SAMPLE_TIME */
  uint64_t stream_id; /* if PERF_SAMPLE_STREAM_ID */
  uint32_t cpu, res;  /* if PERF_SAMPLE_CPU */
};

struct __attribute__((__packed__)) RingBufferForkExit {
  perf_event_header header;
  uint32_t pid, ppid;
  uint32_t tid, ptid;
  uint64_t time;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
};

// This struct must be in sync with the `kSampleRegsUserAll` in PerfEventOpen.h.
struct __attribute__((__packed__)) RingBufferSampleRegsUserAll {
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

// This struct must be in sync with the `kSampleRegsUserAx` in PerfEventOpen.h.
struct __attribute__((__packed__)) RingBufferSampleRegsUserAx {
  uint64_t abi;
  uint64_t ax;
};

// This struct must be in sync with the `kSampleRegsUserSpIp` in PerfEventOpen.h.
struct __attribute__((__packed__)) RingBufferSampleRegsUserSpIp {
  uint64_t abi;
  uint64_t sp;
  uint64_t ip;
};

// This struct must be in sync with the `kSampleRegsUserSp` in PerfEventOpen.h.
struct __attribute__((__packed__)) RingBufferSampleRegsUserSp {
  uint64_t sp;
};

// This struct must be in sync with the `kSampleRegsUserSpIpArguments` in PerfEventOpen.h.
struct __attribute__((__packed__)) RingBufferSampleRegsUserSpIpArguments {
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

struct __attribute__((__packed__)) RingBufferSampleStackUser8bytes {
  uint64_t size;
  uint64_t top8bytes;
  uint64_t dyn_size;
};

struct __attribute__((__packed__)) RingBufferStackSampleFixed {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  uint64_t abi;
  RingBufferSampleRegsUserAll regs;
  // Following this field there are the following fields, which we read dynamically:
  // uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  // char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  // uint64_t dyn_size;                 /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

struct __attribute__((__packed__)) RingBufferSpIpArguments8bytesSample {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  RingBufferSampleRegsUserSpIpArguments regs;
  RingBufferSampleStackUser8bytes stack;
};

struct __attribute__((__packed__)) RingBufferSpIp8bytesSample {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  RingBufferSampleRegsUserSpIp regs;
  RingBufferSampleStackUser8bytes stack;
};

struct __attribute__((__packed__)) RingBufferSpStackUserSampleFixed {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  uint64_t abi;
  RingBufferSampleRegsUserSp regs;
  // Following this field there are the following fields, which we read dynamically:
  // uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  // char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  // uint64_t dyn_size;                 /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

struct __attribute__((__packed__)) RingBufferEmptySample {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
};

struct __attribute__((__packed__)) RingBufferAxSample {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  RingBufferSampleRegsUserAx regs;
};

template <typename TracepointT>
struct __attribute__((__packed__)) RingBufferRawSample {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  uint32_t size;
  TracepointT data;
};

struct __attribute__((__packed__)) RingBufferRawSampleFixed {
  perf_event_header header;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
  uint32_t size;
  // The rest of the sample is a char[size] that we read dynamically.
};

struct __attribute__((__packed__)) RingBufferMmapUpToPgoff {
  perf_event_header header;
  uint32_t pid;
  uint32_t tid;
  uint64_t address;
  uint64_t length;
  uint64_t page_offset;
  // OMITTED: char filename[]
  // OMITTED: RingBufferSampleIdTidTimeStreamidCpu sample_id;
};

struct __attribute__((__packed__)) RingBufferLost {
  perf_event_header header;
  uint64_t id;
  uint64_t lost;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
};

struct __attribute__((__packed__)) RingBufferThrottleUnthrottle {
  perf_event_header header;
  uint64_t time;
  uint64_t id;
  uint64_t lost;
  RingBufferSampleIdTidTimeStreamidCpu sample_id;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_RECORDS_H_
