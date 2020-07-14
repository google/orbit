// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_TRACEPOINTS_H_
#define ORBIT_LINUX_TRACING_TRACEPOINTS_H_

#include <cstdint>

// Format described in /sys/kernel/debug/tracing/events/<category>/<name>/format

struct __attribute__((__packed__)) tracepoint_common {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
};

struct __attribute__((__packed__)) task_rename_tracepoint {
  tracepoint_common common;
  int32_t pid;
  char oldcomm[16];
  char newcomm[16];
  int16_t oom_score_adj;
};

#endif  // ORBIT_LINUX_TRACING_TRACEPOINTS_H_
