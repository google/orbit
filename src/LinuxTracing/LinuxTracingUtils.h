// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LINUX_TRACING_UTILS_H_
#define LINUX_TRACING_LINUX_TRACING_UTILS_H_

#include <absl/types/span.h>
#include <stdint.h>
#include <unistd.h>

#include <ctime>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "ModuleUtils/ReadLinuxMaps.h"

namespace orbit_linux_tracing {

// The association between a character and a thread state is documented at
// https://man7.org/linux/man-pages/man5/proc.5.html in the "/proc/[pid]/stat" section,
// and at https://www.man7.org/linux/man-pages/man1/ps.1.html#PROCESS_STATE_CODES.
std::optional<char> GetThreadState(pid_t tid);

int GetNumCores();

std::optional<std::string> ExtractCpusetFromCgroup(std::string_view cgroup_content);

std::vector<int> ParseCpusetCpus(std::string_view cpuset_cpus_content);

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
  // POSIX guarantees the result to be greater or equal than 1. So we can safely cast here.
  return static_cast<size_t>(sysconf(_SC_PAGESIZE));
}

// Determines whether a function can be instrumented with uprobes given the content of
// /proc/[pid]/maps and the list of modules loaded by the process. Remember that uprobes requires
// the function to be loaded with a file mapping.
// Returns an std::map (so that the order by function id is preserved) from function id to a message
// describing the issue for that function.
[[nodiscard]] std::map<uint64_t, std::string> FindFunctionsThatUprobesCannotInstrumentWithMessages(
    absl::Span<const orbit_module_utils::LinuxMemoryMapping> maps,
    absl::Span<const orbit_grpc_protos::ModuleInfo> modules,
    absl::Span<const orbit_grpc_protos::InstrumentedFunction> functions);

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LINUX_TRACING_UTILS_H_
