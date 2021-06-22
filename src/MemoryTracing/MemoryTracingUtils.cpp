// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracingUtils.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_cat.h>
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

SystemMemoryUsage CreateAndInitializeSystemMemoryUsage() {
  SystemMemoryUsage system_memory_usage;
  system_memory_usage.set_total_kb(kMissingInfo);
  system_memory_usage.set_free_kb(kMissingInfo);
  system_memory_usage.set_available_kb(kMissingInfo);
  system_memory_usage.set_buffers_kb(kMissingInfo);
  system_memory_usage.set_cached_kb(kMissingInfo);
  system_memory_usage.set_pgfault(kMissingInfo);
  system_memory_usage.set_pgmajfault(kMissingInfo);
  return system_memory_usage;
}

ErrorMessageOr<void> UpdateSystemMemoryUsageFromMemInfo(std::string_view meminfo_content,
                                                        SystemMemoryUsage* system_memory_usage) {
  if (meminfo_content.empty()) return ErrorMessage("Empty file content.");

  std::vector<std::string> lines = absl::StrSplit(meminfo_content, '\n', absl::SkipEmpty());
  constexpr size_t kNumLines = 5;
  std::vector<std::string> top_lines(
      lines.begin(), lines.begin() + (lines.size() > kNumLines ? kNumLines : lines.size()));
  std::string error_message;
  for (std::string_view line : top_lines) {
    // Each line of the /proc/meminfo file consists of a parameter name, followed by a colon, the
    // value of the parameter, and an option unit of measurement (e.g., "kB"). According to the
    // kernel code https://github.com/torvalds/linux/blob/master/fs/proc/meminfo.c, the size unit in
    // /proc/meminfo is fixed to "kB", which implies 1024 Bytes. And this is different from the
    // definition in http://en.wikipedia.org/wiki/Kilobyte. We keep consistent with the definition
    // in /proc/meminfo: we report in "kB" and consider 1 kB = 1 KiloBytes = 1024 Bytes.
    // If the line format is wrong or the unit size isn't "kB", SystemMemoryUsage won't be updated.
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 3 || splits[2] != "kB") {
      absl::StrAppend(&error_message, "Wrong format in line: ", line, "\n");
      continue;
    }

    int64_t memory_size_value;
    if (!absl::SimpleAtoi(splits[1], &memory_size_value)) {
      absl::StrAppend(&error_message, "Fail to extract value in line: ", line, "\n");
      continue;
    }

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

  if (!error_message.empty()) return ErrorMessage(error_message);
  return outcome::success();
}

ErrorMessageOr<void> UpdateSystemMemoryUsageFromVmStat(std::string_view vmstat_content,
                                                       SystemMemoryUsage* system_memory_usage) {
  if (vmstat_content.empty()) return ErrorMessage("Empty file content.");

  std::vector<std::string> lines = absl::StrSplit(vmstat_content, '\n', absl::SkipEmpty());
  std::string error_message;
  for (std::string_view line : lines) {
    // Each line of the /proc/vmstat file consists a single name-value pair, delimited by white
    // space. In /proc/vmstat, the pgfault and pgmajfault fields report cumulative values.
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 2) {
      absl::StrAppend(&error_message, "Wrong format in line: ", line, "\n");
      continue;
    }

    int64_t value;
    if (!absl::SimpleAtoi(splits[1], &value)) {
      absl::StrAppend(&error_message, "Fail to extract value in line: ", line, "\n");
      continue;
    }

    if (splits[0] == "pgfault") {
      system_memory_usage->set_pgfault(value);
    } else if (splits[0] == "pgmajfault") {
      system_memory_usage->set_pgmajfault(value);
    }
  }

  if (!error_message.empty()) return ErrorMessage(error_message);
  return outcome::success();
}

ErrorMessageOr<SystemMemoryUsage> GetSystemMemoryUsage() noexcept {
  SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
  system_memory_usage.set_timestamp_ns(orbit_base::CaptureTimestampNs());

  const std::string kSystemMemoryUsageFilename = "/proc/meminfo";
  ErrorMessageOr<std::string> reading_result =
      orbit_base::ReadFileToString(kSystemMemoryUsageFilename);
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  ErrorMessageOr<void> updating_result =
      UpdateSystemMemoryUsageFromMemInfo(reading_result.value(), &system_memory_usage);
  if (updating_result.has_error()) {
    ERROR("Error while updating SystemMemoryUsage from %s: %s", kSystemMemoryUsageFilename,
          updating_result.error().message());
  }

  const std::string kSystemPageFaultsFilename = "/proc/vmstat";
  reading_result = orbit_base::ReadFileToString(kSystemPageFaultsFilename);
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  updating_result = UpdateSystemMemoryUsageFromVmStat(reading_result.value(), &system_memory_usage);
  if (updating_result.has_error()) {
    ERROR("Error while updating SystemMemoryUsage from %s: %s", kSystemPageFaultsFilename,
          updating_result.error().message());
  }

  return system_memory_usage;
}

ProcessMemoryUsage CreateAndInitializeProcessMemoryUsage() {
  ProcessMemoryUsage process_memory_usage;
  process_memory_usage.set_rss_anon_kb(kMissingInfo);
  process_memory_usage.set_minflt(kMissingInfo);
  process_memory_usage.set_majflt(kMissingInfo);
  return process_memory_usage;
}

ErrorMessageOr<void> UpdateProcessMemoryUsageFromProcessStat(
    std::string_view stat_content, ProcessMemoryUsage* process_memory_usage) {
  if (stat_content.empty()) return ErrorMessage("Empty file content.");

  // According to the kernel code
  // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/fs/proc/array.c,
  // the /proc/<PID>/stat file records 52 process status information on a single line, in a fixed
  // order. We are interested in the following fields:
  //   Field index | Name   | Format | Meaning
  //    10         | minflt | %lu    | # of minor faults the process has made
  //    12         | majflt | %lu    | # of major faults the process has made
  std::vector<std::string> splits = absl::StrSplit(stat_content, ' ', absl::SkipWhitespace{});
  if (splits.size() != 52) {
    return ErrorMessage(absl::StrFormat("Wrong format: only %d fields", splits.size()));
  }

  int64_t value;
  std::string error_message;
  if (absl::SimpleAtoi(splits[9], &value)) {
    process_memory_usage->set_minflt(value);
  } else {
    absl::StrAppend(&error_message, "Fail to extract minflt value from: ", splits[9], "\n");
  }

  if (absl::SimpleAtoi(splits[11], &value)) {
    process_memory_usage->set_majflt(value);
  } else {
    absl::StrAppend(&error_message, "Fail to extract majflt value from: ", splits[11], "\n");
  }

  if (!error_message.empty()) return ErrorMessage(error_message);
  return outcome::success();
}

ErrorMessageOr<void> UpdateProcessMemoryUsageFromProcessStatus(
    std::string_view status_content, ProcessMemoryUsage* process_memory_usage) {
  if (status_content.empty()) return ErrorMessage("Empty file content.");

  std::vector<std::string> lines = absl::StrSplit(status_content, '\n', absl::SkipEmpty());
  for (std::string_view line : lines) {
    std::vector<std::string> splits =
        absl::StrSplit(line, absl::ByAnyChar(": \t"), absl::SkipWhitespace{});
    if (splits[0] == "RssAnon") {
      if (splits.size() < 3 || splits[2] != "kB") {
        return ErrorMessage(absl::StrFormat("Wrong format in line: %s\n", line));
      }

      int64_t value;
      if (!absl::SimpleAtoi(splits[1], &value)) {
        return ErrorMessage(absl::StrFormat("Fail to extract value in line: %s\n", line));
      }

      process_memory_usage->set_rss_anon_kb(value);
      return outcome::success();
    }
  }

  return ErrorMessage("RssAnon value not found in the file content.");
}

ErrorMessageOr<ProcessMemoryUsage> GetProcessMemoryUsage(int32_t pid) noexcept {
  ProcessMemoryUsage process_memory_usage = CreateAndInitializeProcessMemoryUsage();
  process_memory_usage.set_pid(pid);
  process_memory_usage.set_timestamp_ns(orbit_base::CaptureTimestampNs());

  const std::string kProcessPageFaultsFilename = absl::StrFormat("/proc/%d/stat", pid);
  ErrorMessageOr<std::string> reading_result =
      orbit_base::ReadFileToString(kProcessPageFaultsFilename);
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  ErrorMessageOr<void> updating_result =
      UpdateProcessMemoryUsageFromProcessStat(reading_result.value(), &process_memory_usage);
  if (updating_result.has_error()) {
    ERROR("Error while updating ProcessMemoryUsage from %s: %s", kProcessPageFaultsFilename,
          updating_result.error().message());
  }

  const std::string kProcessMemoryUsageFilename = absl::StrFormat("/proc/%d/status", pid);
  reading_result = orbit_base::ReadFileToString(kProcessMemoryUsageFilename);
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  updating_result =
      UpdateProcessMemoryUsageFromProcessStatus(reading_result.value(), &process_memory_usage);
  if (updating_result.has_error()) {
    ERROR("Error while updating ProcessMemoryUsage from %s: %s", kProcessMemoryUsageFilename,
          updating_result.error().message());
  }

  return process_memory_usage;
}

CGroupMemoryUsage CreateAndInitializeCGroupMemoryUsage() {
  CGroupMemoryUsage cgroup_memory_usage;
  cgroup_memory_usage.set_limit_bytes(kMissingInfo);
  cgroup_memory_usage.set_rss_bytes(kMissingInfo);
  cgroup_memory_usage.set_mapped_file_bytes(kMissingInfo);
  cgroup_memory_usage.set_pgfault(kMissingInfo);
  cgroup_memory_usage.set_pgmajfault(kMissingInfo);
  cgroup_memory_usage.set_unevictable_bytes(kMissingInfo);
  cgroup_memory_usage.set_inactive_anon_bytes(kMissingInfo);
  cgroup_memory_usage.set_active_anon_bytes(kMissingInfo);
  cgroup_memory_usage.set_inactive_file_bytes(kMissingInfo);
  cgroup_memory_usage.set_active_file_bytes(kMissingInfo);
  return cgroup_memory_usage;
}

std::string GetProcessMemoryCGroupName(std::string_view cgroup_content) {
  if (cgroup_content.empty()) return "";

  std::vector<std::string> lines = absl::StrSplit(cgroup_content, '\n', absl::SkipEmpty());
  for (std::string_view line : lines) {
    std::vector<std::string> splits = absl::StrSplit(line, absl::MaxSplits(':', 2));
    // If find the memory cgroup, return the cgroup name without the leading "/".
    if (splits.size() == 3 && splits[1] == "memory") return splits[2].substr(1);
  }

  return "";
}

ErrorMessageOr<void> UpdateCGroupMemoryUsageFromMemoryLimitInBytes(
    std::string_view memory_limit_in_bytes_content, CGroupMemoryUsage* cgroup_memory_usage) {
  if (memory_limit_in_bytes_content.empty()) return ErrorMessage("Empty file content.");

  // The memory.limit_in_bytes file use "bytes" as the size unit.
  int64_t memory_limit_in_bytes;
  if (absl::SimpleAtoi(memory_limit_in_bytes_content, &memory_limit_in_bytes)) {
    cgroup_memory_usage->set_limit_bytes(memory_limit_in_bytes);
    return outcome::success();
  } else {
    return ErrorMessage(
        absl::StrFormat("Fail to extract limit value from: %s", memory_limit_in_bytes_content));
  }
}

ErrorMessageOr<void> UpdateCGroupMemoryUsageFromMemoryStat(std::string_view memory_stat_content,
                                                           CGroupMemoryUsage* cgroup_memory_usage) {
  if (memory_stat_content.empty()) return ErrorMessage("Empty file content.");

  std::vector<std::string> lines = absl::StrSplit(memory_stat_content, '\n', absl::SkipEmpty());
  std::string error_message;
  for (std::string_view line : lines) {
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    // According to the document https://www.kernel.org/doc/Documentation/cgroup-v1/memory.txt:
    // Each line of the memory.stat file consists of a parameter name, followed by a whitespace,
    // and the value of the parameter. Also the memory size unit is fixed to "bytes".
    if (splits.size() < 2) {
      absl::StrAppend(&error_message, "Wrong format in line: ", line, "\n");
      continue;
    }

    int64_t value;
    if (!absl::SimpleAtoi(splits[1], &value)) {
      absl::StrAppend(&error_message, "Fail to extract value in line: ", line, "\n");
      continue;
    }

    if (splits[0] == "rss") {
      cgroup_memory_usage->set_rss_bytes(value);
    } else if (splits[0] == "mapped_file") {
      cgroup_memory_usage->set_mapped_file_bytes(value);
    } else if (splits[0] == "pgfault") {
      cgroup_memory_usage->set_pgfault(value);
    } else if (splits[0] == "pgmajfault") {
      cgroup_memory_usage->set_pgmajfault(value);
    } else if (splits[0] == "unevictable") {
      cgroup_memory_usage->set_unevictable_bytes(value);
    } else if (splits[0] == "inactive_anon") {
      cgroup_memory_usage->set_inactive_anon_bytes(value);
    } else if (splits[0] == "active_anon") {
      cgroup_memory_usage->set_active_anon_bytes(value);
    } else if (splits[0] == "inactive_file") {
      cgroup_memory_usage->set_inactive_file_bytes(value);
    } else if (splits[0] == "active_file") {
      cgroup_memory_usage->set_active_file_bytes(value);
    }
  }

  if (!error_message.empty()) return ErrorMessage(error_message);
  return outcome::success();
}

ErrorMessageOr<CGroupMemoryUsage> GetCGroupMemoryUsage(int32_t pid) noexcept {
  uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  const std::string kProcessCGroupsFilename = absl::StrFormat("/proc/%d/cgroup", pid);
  ErrorMessageOr<std::string> reading_result =
      orbit_base::ReadFileToString(kProcessCGroupsFilename);
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

  CGroupMemoryUsage cgroup_memory_usage = CreateAndInitializeCGroupMemoryUsage();
  cgroup_memory_usage.set_cgroup_name(cgroup_name);
  cgroup_memory_usage.set_timestamp_ns(current_timestamp_ns);

  const std::string kCGroupMemoryLimitFilename =
      absl::StrFormat("/sys/fs/cgroup/memory/%s/memory.limit_in_bytes", cgroup_name);
  reading_result = orbit_base::ReadFileToString(kCGroupMemoryLimitFilename);
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  ErrorMessageOr<void> updating_result =
      UpdateCGroupMemoryUsageFromMemoryLimitInBytes(reading_result.value(), &cgroup_memory_usage);
  if (updating_result.has_error()) {
    ERROR("Error while updating CGroupMemoryUsage from %s: %s", kCGroupMemoryLimitFilename,
          updating_result.error().message());
  }

  const std::string kCGroupMemoryUsageAndPageFaultsFilename =
      absl::StrFormat("/sys/fs/cgroup/memory/%s/memory.stat", cgroup_name);
  reading_result = orbit_base::ReadFileToString(kCGroupMemoryUsageAndPageFaultsFilename);
  if (reading_result.has_error()) {
    ERROR("%s", reading_result.error().message());
    return reading_result.error();
  }
  updating_result =
      UpdateCGroupMemoryUsageFromMemoryStat(reading_result.value(), &cgroup_memory_usage);
  if (updating_result.has_error()) {
    ERROR("Error while updating CGroupMemoryUsage from %s: %s",
          kCGroupMemoryUsageAndPageFaultsFilename, updating_result.error().message());
  }

  return cgroup_memory_usage;
}

}  // namespace orbit_memory_tracing