#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_RECORDS_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_RECORDS_H_

#include "PerfEventOpen.h"

namespace LinuxTracing {

// This struct must be in sync with the SAMPLE_TYPE_TID_TIME_STREAMID_CPU in
// PerfEventOpen.h, as the bits set in perf_event_attr::sample_type determine
// the fields this struct should have.
struct __attribute__((__packed__)) perf_event_sample_id_tid_time_streamid_cpu {
  uint32_t pid, tid;  /* if PERF_SAMPLE_TID */
  uint64_t time;      /* if PERF_SAMPLE_TIME */
  uint64_t stream_id; /* if PERF_SAMPLE_STREAM_ID */
  uint32_t cpu, res;  /* if PERF_SAMPLE_CPU */
};

struct __attribute__((__packed__)) perf_event_context_switch {
  perf_event_header header;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
};

struct __attribute__((__packed__)) perf_event_context_switch_cpu_wide {
  perf_event_header header;
  uint32_t next_prev_pid;
  uint32_t next_prev_tid;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
};

struct __attribute__((__packed__)) perf_event_fork_exit {
  perf_event_header header;
  uint32_t pid, ppid;
  uint32_t tid, ptid;
  uint64_t time;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
};

// This struct must be in sync with the SAMPLE_REGS_USER_ALL in
// PerfEventOpen.h,
struct __attribute__((__packed__)) perf_event_sample_regs_user_all {
  uint64_t abi;
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

struct __attribute__((__packed__)) perf_event_sample_stack_user {
  uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  uint64_t dyn_size; /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

struct __attribute__((__packed__)) perf_event_empty_sample {
  perf_event_header header;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
};

struct __attribute__((__packed__)) perf_event_stack_sample {
  perf_event_header header;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  perf_event_sample_regs_user_all regs;
  perf_event_sample_stack_user stack;
};

struct __attribute__((__packed__)) perf_event_lost {
  perf_event_header header;
  uint64_t id;
  uint64_t lost;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
};

// Tracepoints are perf samples, so they start with the same layout as a regular
// perf sample, that is, with the header and the common fields like sample_id,
// tid, timestamp, etc. The next field is the size of the rest of the tracepoint
// record and the common type. The common type is the same as the tracepoint id
// and we use it to identify the end events and how to handle them. The actual
// record is thus larger than the size of this struct but since it is dynamic
// and depends on the type of the tracepoint, we only hardcode the common part
// here.
struct __attribute__((__packed__)) perf_event_tracepoint {
  perf_event_header header;
  perf_event_sample_id_tid_time_cpu sample_id;
  uint32_t size;
  uint16_t common_type;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_RECORDS_H_
