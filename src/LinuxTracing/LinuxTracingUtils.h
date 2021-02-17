// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LINUX_TRACING_UTILS_H_
#define LINUX_TRACING_LINUX_TRACING_UTILS_H_

#include <unistd.h>

#include <ctime>
#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

namespace orbit_linux_tracing {

std::string ReadMaps(pid_t pid);

// The association between a character and a thread state is documented at
// https://man7.org/linux/man-pages/man5/proc.5.html in the "/proc/[pid]/stat" section,
// and at https://www.man7.org/linux/man-pages/man1/ps.1.html#PROCESS_STATE_CODES.
std::optional<char> GetThreadState(pid_t tid);

int GetNumCores();

std::optional<std::string> ExtractCpusetFromCgroup(const std::string& cgroup_content);

std::vector<int> ParseCpusetCpus(const std::string& cpuset_cpus_content);

std::vector<int> GetCpusetCpus(pid_t pid);

// Looks up the tracepoint id for the given category (example: "sched")
// and name (example: "sched_waking"). Returns the tracepoint id or
// -1 in case of any errors.
int GetTracepointId(const char* tracepoint_category, const char* tracepoint_name);

uint64_t GetMaxOpenFilesHardLimit();

bool SetMaxOpenFilesSoftLimit(uint64_t soft_limit);

#if defined(__x86_64__)

#define READ_ONCE(x) (*static_cast<volatile typeof(x)*>(&x))
#define WRITE_ONCE(x, v) (*static_cast<volatile typeof(x)*>(&x)) = (v)
#define barrier() asm volatile("" ::: "memory")

#define smp_store_release(p, v) \
  do {                          \
    barrier();                  \
    WRITE_ONCE(*p, v);          \
  } while (0)

#define smp_load_acquire(p)          \
  ({                                 \
    typeof(*p) ___p = READ_ONCE(*p); \
    barrier();                       \
    ___p;                            \
  })

#endif

inline size_t GetPageSize() {
  // POSIX guarantees the result to be greater or equal than 1.
  // So we can safely cast here.
  return static_cast<size_t>(sysconf(_SC_PAGESIZE));
}
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LINUX_TRACING_UTILS_H_
