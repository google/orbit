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

SystemMemoryUsage ParseMemInfo(std::string_view meminfo_content) {
  SystemMemoryUsage memory_info;
  memory_info.set_total_kb(kMissingInfo);
  memory_info.set_free_kb(kMissingInfo);
  memory_info.set_available_kb(kMissingInfo);
  memory_info.set_buffers_kb(kMissingInfo);
  memory_info.set_cached_kb(kMissingInfo);

  if (meminfo_content.empty()) return memory_info;

  std::vector<std::string> lines = absl::StrSplit(meminfo_content, '\n');
  if (lines.empty()) return memory_info;

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
      memory_info.set_total_kb(memory_size_value);
    } else if (splits[0] == "MemFree:") {
      memory_info.set_free_kb(memory_size_value);
    } else if (splits[0] == "MemAvailable:") {
      memory_info.set_available_kb(memory_size_value);
    } else if (splits[0] == "Buffers:") {
      memory_info.set_buffers_kb(memory_size_value);
    } else if (splits[0] == "Cached:") {
      memory_info.set_cached_kb(memory_size_value);
    }
  }

  return memory_info;
}

ErrorMessageOr<SystemMemoryUsage> GetSystemMemoryUsage() noexcept {
  uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  ErrorMessageOr<std::string> reading_result = orbit_base::ReadFileToString("/proc/meminfo");
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }

  SystemMemoryUsage parsing_result = ParseMemInfo(reading_result.value());
  parsing_result.set_timestamp_ns(current_timestamp_ns);
  return parsing_result;
}

int64_t GetVmRssFromProcessStatus(std::string_view status_content) {
  if (status_content.empty()) return kMissingInfo;

  std::vector<std::string> lines = absl::StrSplit(status_content, '\n');
  if (lines.empty()) return kMissingInfo;

  for (std::string_view line : lines) {
    if (absl::StartsWith(line, "VmRSS")) {
      std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
      // According to the kernel code
      // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/fs/proc/task_mmu.c,
      // the size unit of the VmRSS field in the /proc/<PID>/status file is fixed to "kB", which
      // implies 1024 Bytes. We report in "KB" in ProcessSystemUsage.
      int64_t rss_kb;
      if (splits.size() == 3 && splits[2] == "kB" && absl::SimpleAtoi(splits[1], &rss_kb)) {
        return rss_kb;
      }
      return kMissingInfo;
    }
  }

  return kMissingInfo;
}

ErrorMessageOr<ProcessMemoryUsage> GetProcessMemoryUsage(uint32_t pid) noexcept {
  uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  // Extract the VmRSS value in kilobytes from the /proc/<pid>/status file.
  ErrorMessageOr<std::string> reading_result =
      orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/status", pid));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  int64_t vm_rss_kb = GetVmRssFromProcessStatus(reading_result.value());

  ProcessMemoryUsage process_memory_usage;
  process_memory_usage.set_pid(pid);
  process_memory_usage.set_timestamp_ns(current_timestamp_ns);
  process_memory_usage.set_vm_rss_kb(vm_rss_kb);

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

int64_t GetCGroupMemoryLimitInBytes(std::string_view memory_limit_in_bytes_content) {
  if (memory_limit_in_bytes_content.empty()) return kMissingInfo;

  // The memory.limit_in_bytes file use "bytes" as the size unit.
  int64_t memory_limit_in_bytes;
  if (absl::SimpleAtoi(memory_limit_in_bytes_content, &memory_limit_in_bytes)) {
    return memory_limit_in_bytes;
  }

  return kMissingInfo;
}

int64_t GetRssFromCGroupMemoryStat(std::string_view memory_stat_content) {
  if (memory_stat_content.empty()) return kMissingInfo;

  std::vector<std::string> lines = absl::StrSplit(memory_stat_content, '\n');
  if (lines.empty()) return kMissingInfo;

  for (std::string_view line : lines) {
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() == 2 && splits[0] == "rss") {
      // According to the document https://www.kernel.org/doc/Documentation/cgroup-v1/memory.txt,
      // the memory.stat file use "bytes" as the size unit.
      int64_t rss_in_bytes;
      if (absl::SimpleAtoi(splits[1], &rss_in_bytes)) return rss_in_bytes;
      return kMissingInfo;
    }
  }

  return kMissingInfo;
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

  // Extract cgroup memory limit from /sys/fs/cgroup/memory/<cgroup_name>/memory.limit_in_bytes.
  reading_result = orbit_base::ReadFileToString(
      absl::StrFormat("/sys/fs/cgroup/memory/%s/memory.limit_in_bytes", cgroup_name));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  int64_t cgroup_memory_limit_in_bytes = GetCGroupMemoryLimitInBytes(reading_result.value());

  // Extract cgroup memory usage from the /sys/fs/cgroup/memory/<cgroup_name>/memory.stat file.
  reading_result = orbit_base::ReadFileToString(
      absl::StrFormat("/sys/fs/cgroup/memory/%s/memory.stat", cgroup_name));
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  int64_t cgroup_rss_in_bytes = GetRssFromCGroupMemoryStat(reading_result.value());

  CGroupMemoryUsage cgroup_memory_usage;
  cgroup_memory_usage.set_cgroup_name(cgroup_name);
  cgroup_memory_usage.set_timestamp_ns(current_timestamp_ns);
  cgroup_memory_usage.set_limit_bytes(cgroup_memory_limit_in_bytes);
  cgroup_memory_usage.set_rss_bytes(cgroup_rss_in_bytes);

  return cgroup_memory_usage;
}

}  // namespace orbit_memory_tracing