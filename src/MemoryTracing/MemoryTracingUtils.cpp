// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracingUtils.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <stdlib.h>

#include <string>

#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_memory_tracing {

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::SystemMemoryUsage;

void GetValuesFromMemInfo(std::string_view meminfo_content,
                          SystemMemoryUsage* system_memory_usage) {
  if (meminfo_content.empty()) return;

  std::vector<std::string> lines = absl::StrSplit(meminfo_content, '\n');
  if (lines.empty()) return;

  constexpr size_t kNumLines = 5;
  std::vector<std::string> top_lines(
      lines.begin(), lines.begin() + (lines.size() > kNumLines ? kNumLines : lines.size()));
  for (std::string_view line : top_lines) {
    // Each line of the /proc/meminfo file consists of a parameter name, followed by a colon, the
    // value of the parameter, and an option unit of measurement (e.g., "kB"). According to the
    // kernel code https://github.com/torvalds/linux/blob/master/fs/proc/meminfo.c, the size unit in
    // /proc/meminfo is fixed to "kB", which implies 1024 Bytes. And this is different from the
    // definition in http://en.wikipedia.org/wiki/Kilobyte. We keep consistent with the definition
    // in /proc/meminfo: we report in "kB" and consider 1 kB = 1 KiloBytes = 1024 Bytes.
    // If the line format is wrong or the unit size isn't "kB", SystemMemoryUsage won't be updated.
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 3 || splits[2] != "kB") continue;

    int64_t memory_size_value;
    if (!absl::SimpleAtoi(splits[1], &memory_size_value)) continue;

    if (splits[0] == "MemTotal:") {
      system_memory_usage->set_total_kb(memory_size_value);
    } else if (splits[0] == "MemFree:") {
      system_memory_usage->set_free_kb(memory_size_value);
    } else if (splits[0] == "MemAvailable:") {
      system_memory_usage->set_available_kb(memory_size_value);
    } else if (splits[0] == "Buffers:") {
      system_memory_usage->set_buffers_kb(memory_size_value);
    } else if (splits[0] == "Cached:") {
      system_memory_usage->set_cached_kb(memory_size_value);
    }
  }
}

void GetValuesFromVmStat(std::string_view vmstat_content, SystemMemoryUsage* system_memory_usage) {
  if (vmstat_content.empty()) return;

  std::vector<std::string> lines = absl::StrSplit(vmstat_content, '\n');
  if (lines.empty()) return;

  for (std::string_view line : lines) {
    // Each line of the /proc/vmstat file consists a single name-value pair, delimited by white
    // space. In /proc/vmstat, the pgfault and pgmajfault fields report cumulative values.
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 2) continue;

    int64_t value;
    if (!absl::SimpleAtoi(splits[1], &value)) continue;

    if (splits[0] == "pgfault") {
      system_memory_usage->set_pgfault(value);
    } else if (splits[0] == "pgmajfault") {
      system_memory_usage->set_pgmajfault(value);
    }
  }
}

ErrorMessageOr<SystemMemoryUsage> GetSystemMemoryUsage() noexcept {
  SystemMemoryUsage system_memory_usage;
  system_memory_usage.set_timestamp_ns(orbit_base::CaptureTimestampNs());
  system_memory_usage.set_total_kb(kMissingInfo);
  system_memory_usage.set_free_kb(kMissingInfo);
  system_memory_usage.set_available_kb(kMissingInfo);
  system_memory_usage.set_buffers_kb(kMissingInfo);
  system_memory_usage.set_cached_kb(kMissingInfo);
  system_memory_usage.set_pgfault(kMissingInfo);
  system_memory_usage.set_pgmajfault(kMissingInfo);

  // Extract system-wide memory usage from the /proc/meminfo file.
  ErrorMessageOr<std::string> reading_result = orbit_base::ReadFileToString("/proc/meminfo");
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  GetValuesFromMemInfo(reading_result.value(), &system_memory_usage);

  // Extract system-wide page fault statistics from the /proc/vmstat file.
  reading_result = orbit_base::ReadFileToString("/proc/vmstat");
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  GetValuesFromVmStat(reading_result.value(), &system_memory_usage);

  return system_memory_usage;
}

void GetValuesFromProcessStat(std::string_view stat_content,
                              ProcessMemoryUsage* process_memory_usage) {
  if (stat_content.empty()) return;

  // According to the kernel code
  // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/fs/proc/array.c,
  // the /proc/<PID>/stat file records 52 process status information on a single line, in a fixed
  // order. We are intersted in the following fields:
  //   Field index | Name   | Format | Meaning
  //    24         | rss    | %ld    | # of pages the process has in real memory
  //    10         | minflt | %lu    | # of minor faults the process has made
  //    12         | majflt | %lu    | # of major faults the process has made
  std::vector<std::string> splits = absl::StrSplit(stat_content, ' ', absl::SkipWhitespace{});
  if (splits.size() != 52) return;

  int64_t value;
  if (absl::SimpleAtoi(splits[23], &value)) process_memory_usage->set_rss_pages(value);
  if (absl::SimpleAtoi(splits[9], &value)) process_memory_usage->set_minflt(value);
  if (absl::SimpleAtoi(splits[11], &value)) process_memory_usage->set_majflt(value);
}

ErrorMessageOr<ProcessMemoryUsage> GetProcessMemoryUsage(uint32_t pid) noexcept {
  ProcessMemoryUsage process_memory_usage;
  process_memory_usage.set_pid(pid);
  process_memory_usage.set_timestamp_ns(orbit_base::CaptureTimestampNs());

  // Extract the rss value (in pages) and page fault statistics from the /proc/<pid>/status file.
  ErrorMessageOr<std::string> reading_result =
      orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/stat", pid));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  GetValuesFromProcessStat(reading_result.value(), &process_memory_usage);

  return process_memory_usage;
}

std::string GetProcessMemoryCGroupName(std::string_view cgroup_content) {
  if (cgroup_content.empty()) return "";

  std::vector<std::string> lines = absl::StrSplit(cgroup_content, '\n');
  if (lines.empty()) return "";

  for (std::string_view line : lines) {
    std::vector<std::string> splits = absl::StrSplit(line, absl::MaxSplits(':', 2));
    // If find the memory cgroup, return the cgroup name without the leading "/".
    if (splits.size() == 3 && splits[1] == "memory") return splits[2].substr(1);
  }

  return "";
}

void GetCGroupMemoryLimitInBytes(std::string_view memory_limit_in_bytes_content,
                                 CGroupMemoryUsage* cgroup_memory_usage) {
  if (memory_limit_in_bytes_content.empty()) return;

  // The memory.limit_in_bytes file use "bytes" as the size unit.
  int64_t memory_limit_in_bytes;
  if (absl::SimpleAtoi(memory_limit_in_bytes_content, &memory_limit_in_bytes)) {
    cgroup_memory_usage->set_limit_bytes(memory_limit_in_bytes);
  }
}

void GetValuesFromCGroupMemoryStat(std::string_view memory_stat_content,
                                   CGroupMemoryUsage* cgroup_memory_usage) {
  if (memory_stat_content.empty()) return;

  std::vector<std::string> lines = absl::StrSplit(memory_stat_content, '\n');
  if (lines.empty()) return;

  for (std::string_view line : lines) {
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    // According to the document https://www.kernel.org/doc/Documentation/cgroup-v1/memory.txt:
    // Each line of the memory.stat file consists of a parameter name, followed by a whitespace, and
    // the value of the parameter. Also the memory size unit is fixed to "bytes".
    if (splits.size() < 2) continue;

    int64_t value;
    if (!absl::SimpleAtoi(splits[1], &value)) continue;

    if (splits[0] == "rss") {
      cgroup_memory_usage->set_rss_bytes(value);
    } else if (splits[0] == "mapped_file") {
      cgroup_memory_usage->set_mapped_file_bytes(value);
    } else if (splits[0] == "pgfault") {
      cgroup_memory_usage->set_pgfault(value);
    } else if (splits[0] == "pgmajfault") {
      cgroup_memory_usage->set_pgmajfault(value);
    }
  }
}

ErrorMessageOr<CGroupMemoryUsage> GetCGroupMemoryUsage(uint32_t pid) noexcept {
  uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  // Extract cgroup name from the /proc/<pid>/cgroup file.
  ErrorMessageOr<std::string> reading_result =
      orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/cgroup", pid));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  std::string cgroup_name = GetProcessMemoryCGroupName(reading_result.value());
  if (cgroup_name.empty()) {
    std::string error_message =
        absl::StrFormat("Fail to extract the cgroup name of the target process %d.", pid);
    ERROR("%s", error_message);
    return ErrorMessage{std::move(error_message)};
  }

  CGroupMemoryUsage cgroup_memory_usage;
  cgroup_memory_usage.set_cgroup_name(cgroup_name);
  cgroup_memory_usage.set_timestamp_ns(current_timestamp_ns);
  cgroup_memory_usage.set_limit_bytes(kMissingInfo);
  cgroup_memory_usage.set_rss_bytes(kMissingInfo);
  cgroup_memory_usage.set_mapped_file_bytes(kMissingInfo);

  // Extract cgroup memory limit from /sys/fs/cgroup/memory/<cgroup_name>/memory.limit_in_bytes.
  reading_result = orbit_base::ReadFileToString(
      absl::StrFormat("/sys/fs/cgroup/memory/%s/memory.limit_in_bytes", cgroup_name));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  GetCGroupMemoryLimitInBytes(reading_result.value(), &cgroup_memory_usage);

  // Extract cgroup memory usage from the /sys/fs/cgroup/memory/<cgroup_name>/memory.stat file.
  reading_result = orbit_base::ReadFileToString(
      absl::StrFormat("/sys/fs/cgroup/memory/%s/memory.stat", cgroup_name));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  GetValuesFromCGroupMemoryStat(reading_result.value(), &cgroup_memory_usage);

  return cgroup_memory_usage;
}

}  // namespace orbit_memory_tracing