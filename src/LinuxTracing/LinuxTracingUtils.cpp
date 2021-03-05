// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>

#include <algorithm>
#include <array>
#include <ctime>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "OrbitBase/ExecuteCommand.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_linux_tracing {

namespace fs = std::filesystem;

std::string ReadMaps(pid_t pid) {
  std::string maps_filename = absl::StrFormat("/proc/%d/maps", pid);
  ErrorMessageOr<std::string> maps_content_or_error = orbit_base::ReadFileToString(maps_filename);
  if (maps_content_or_error.has_error()) {
    return {};
  }

  return maps_content_or_error.value();
}

std::optional<char> GetThreadState(pid_t tid) {
  fs::path stat{fs::path{"/proc"} / std::to_string(tid) / "stat"};
  if (!fs::exists(stat)) {
    return std::nullopt;
  }

  ErrorMessageOr<std::string> file_content_or_error = orbit_base::ReadFileToString(stat);
  if (file_content_or_error.has_error()) {
    ERROR("Could not open \"%s\": %s", stat.string(), file_content_or_error.error().message());
    return std::nullopt;
  }

  std::vector<std::string> lines =
      absl::StrSplit(file_content_or_error.value(), absl::MaxSplits('\n', 1));
  if (lines.empty()) {
    ERROR("Empty \"%s\" file", stat.string());
    return std::nullopt;
  }
  std::string first_line = lines.at(0);

  // Remove fields up to comm (process name) as this, enclosed in parentheses, could contain spaces.
  size_t last_closed_paren_index = first_line.find_last_of(')');
  if (last_closed_paren_index == std::string::npos) {
    return std::nullopt;
  }
  std::string_view first_line_excl_pid_comm =
      std::string_view{first_line}.substr(last_closed_paren_index + 1);

  std::vector<std::string_view> fields_excl_pid_comm =
      absl::StrSplit(first_line_excl_pid_comm, ' ', absl::SkipWhitespace{});

  constexpr size_t kCommIndex = 1;
  constexpr size_t kStateIndex = 2;
  constexpr size_t kStateIndexExclPidComm = kStateIndex - kCommIndex - 1;
  if (fields_excl_pid_comm.size() <= kStateIndexExclPidComm) {
    return std::nullopt;
  }
  return fields_excl_pid_comm[kStateIndexExclPidComm][0];
}

int GetNumCores() {
  int hw_conc = static_cast<int>(std::thread::hardware_concurrency());
  // Some compilers do not support std::thread::hardware_concurrency().
  if (hw_conc != 0) {
    return hw_conc;
  }

  std::optional<std::string> nproc_str = orbit_base::ExecuteCommand("nproc");
  if (nproc_str.has_value() && !nproc_str.value().empty()) {
    return std::stoi(nproc_str.value());
  }

  return 1;
}

// Read /proc/<pid>/cgroup.
static ErrorMessageOr<std::string> ReadCgroupContent(pid_t pid) {
  std::string cgroup_filename = absl::StrFormat("/proc/%d/cgroup", pid);
  return orbit_base::ReadFileToString(cgroup_filename);
}

// Extract the cpuset entry from the content of /proc/<pid>/cgroup.
std::optional<std::string> ExtractCpusetFromCgroup(const std::string& cgroup_content) {
  std::istringstream cgroup_content_ss{cgroup_content};
  std::string cgroup_line;
  while (std::getline(cgroup_content_ss, cgroup_line)) {
    if (cgroup_line.find("cpuset:") != std::string::npos ||
        cgroup_line.find("cpuset,") != std::string::npos) {
      // For example "8:cpuset:/" or "8:cpuset:/game", but potentially also
      // "5:cpuacct,cpu,cpuset:/daemons".
      return cgroup_line.substr(cgroup_line.find_last_of(':') + 1);
    }
  }

  return std::optional<std::string>{};
}

// Read /sys/fs/cgroup/cpuset/<cgroup>/cpuset.cpus.
static ErrorMessageOr<std::string> ReadCpusetCpusContent(const std::string& cgroup_cpuset) {
  std::string cpuset_cpus_filename = absl::StrFormat("/sys/fs/cgroup/cpuset%s/cpuset.cpus",
                                                     cgroup_cpuset == "/" ? "" : cgroup_cpuset);
  return orbit_base::ReadFileToString(cpuset_cpus_filename);
}

std::vector<int> ParseCpusetCpus(const std::string& cpuset_cpus_content) {
  std::vector<int> cpuset_cpus{};
  // Example of format: "0-2,7,12-14".
  for (const auto& range : absl::StrSplit(cpuset_cpus_content, ',', absl::SkipEmpty())) {
    std::vector<std::string> values = absl::StrSplit(range, '-');
    if (values.size() == 1) {
      int cpu = std::stoi(values[0]);
      cpuset_cpus.push_back(cpu);
    } else if (values.size() == 2) {
      for (int cpu = std::stoi(values[0]); cpu <= std::stoi(values[1]); ++cpu) {
        cpuset_cpus.push_back(cpu);
      }
    }
  }
  return cpuset_cpus;
}

// Read and parse /sys/fs/cgroup/cpuset/<cgroup_cpuset>/cpuset.cpus for the
// cgroup cpuset of the process with this pid.
// An empty result indicates an error, as trying to start a process with an
// empty cpuset fails with message "cgroup change of group failed".
std::vector<int> GetCpusetCpus(pid_t pid) {
  ErrorMessageOr<std::string> cgroup_content_or_error = ReadCgroupContent(pid);
  if (cgroup_content_or_error.has_error()) {
    return {};
  }

  // For example "/" or "/game".
  std::optional<std::string> cgroup_cpuset_opt =
      ExtractCpusetFromCgroup(cgroup_content_or_error.value());
  if (!cgroup_cpuset_opt.has_value()) {
    return {};
  }

  // For example "0-2,7,12-14".
  ErrorMessageOr<std::string> cpuset_cpus_content_or_error =
      ReadCpusetCpusContent(cgroup_cpuset_opt.value());
  if (cpuset_cpus_content_or_error.has_error()) {
    return {};
  }

  return ParseCpusetCpus(cpuset_cpus_content_or_error.value());
}

int GetTracepointId(const char* tracepoint_category, const char* tracepoint_name) {
  std::string filename = absl::StrFormat("/sys/kernel/debug/tracing/events/%s/%s/id",
                                         tracepoint_category, tracepoint_name);

  ErrorMessageOr<std::string> file_content_or_error = orbit_base::ReadFileToString(filename);
  if (file_content_or_error.has_error()) {
    ERROR("Reading tracepoint id of %s:%s: %s", tracepoint_category, tracepoint_name,
          file_content_or_error.error().message());
    return -1;
  }

  int tp_id = -1;
  if (!absl::SimpleAtoi(file_content_or_error.value(), &tp_id)) {
    ERROR("Parsing tracepoint id for: %s:%s", tracepoint_category, tracepoint_name);
    return -1;
  }
  return tp_id;
}

uint64_t GetMaxOpenFilesHardLimit() {
  rlimit limit{};
  int ret = getrlimit(RLIMIT_NOFILE, &limit);
  if (ret != 0) {
    ERROR("getrlimit: %s", SafeStrerror(errno));
    return 0;
  }
  return limit.rlim_max;
}

bool SetMaxOpenFilesSoftLimit(uint64_t soft_limit) {
  uint64_t hard_limit = GetMaxOpenFilesHardLimit();
  if (hard_limit == 0) {
    return false;
  }
  rlimit limit{soft_limit, hard_limit};
  int ret = setrlimit(RLIMIT_NOFILE, &limit);
  if (ret != 0) {
    ERROR("setrlimit: %s", SafeStrerror(errno));
    return false;
  }
  return true;
}

}  // namespace orbit_linux_tracing
