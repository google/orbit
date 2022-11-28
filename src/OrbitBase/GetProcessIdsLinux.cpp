// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <sys/types.h>

#include <chrono>
#include <filesystem>
#include <istream>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_base {

namespace fs = std::filesystem;

static std::optional<pid_t> ProcEntryToPid(const std::filesystem::directory_entry& entry) {
  std::error_code error;
  bool is_directory = entry.is_directory(error);
  if (error) {
    ORBIT_ERROR("Unable to stat \"%s\": %s", entry.path(), error.message());
    return std::nullopt;
  }

  if (!is_directory) {
    return std::nullopt;
  }

  int potential_pid;
  if (!absl::SimpleAtoi(entry.path().filename().string(), &potential_pid)) {
    return std::nullopt;
  }

  if (potential_pid <= 0) {
    return std::nullopt;
  }

  return potential_pid;
}

std::vector<pid_t> GetAllPids() {
  std::error_code error;
  fs::directory_iterator proc{"/proc", error};
  if (error) {
    ORBIT_ERROR("Unable to ls /proc: %s", error.message());
    return {};
  }

  std::vector<pid_t> pids;

  for (auto it = fs::begin(proc), end = fs::end(proc); it != end; it.increment(error)) {
    if (error) {
      ORBIT_ERROR("directory_iterator::increment failed with: %s (stopping)", error.message());
      break;
    }
    auto pid = ProcEntryToPid(*it);
    if (pid.has_value()) {
      pids.emplace_back(pid.value());
    }
  }

  return pids;
}

std::vector<pid_t> GetTidsOfProcess(pid_t pid) {
  std::error_code error;
  fs::directory_iterator proc_pid_task{fs::path{"/proc"} / std::to_string(pid) / "task", error};
  if (error) {
    // The process with id `pid` could have stopped existing.
    ORBIT_ERROR("Getting tids of threads of process %d: %s", pid, error.message());
    return {};
  }

  std::vector<pid_t> tids;
  for (auto it = fs::begin(proc_pid_task), end = fs::end(proc_pid_task); it != end;
       it.increment(error)) {
    if (error) {
      ORBIT_ERROR("directory_iterator::increment failed with: %s (stopping)", error.message());
      break;
    }
    if (auto tid = ProcEntryToPid(*it); tid.has_value()) {
      tids.emplace_back(tid.value());
    }
  }
  return tids;
}

ErrorMessageOr<pid_t> GetTracerPidOfProcess(pid_t pid) {
  std::string status_file_name = absl::StrFormat("/proc/%i/status", pid);
  OUTCOME_TRY(auto&& status_file_content, orbit_base::ReadFileToString(status_file_name));
  std::istringstream status_file_content_ss{status_file_content};
  std::string line;
  std::optional<pid_t> tracer_pid;
  constexpr const char* kTracerPidStr = "TracerPid:";

  while (std::getline(status_file_content_ss, line)) {
    if (line.find(kTracerPidStr) == std::string::npos) continue;
    int potential_pid;
    if (!absl::SimpleAtoi(line.substr(line.find_last_of(kTracerPidStr) + 1), &potential_pid)) {
      return ErrorMessage(absl::StrFormat("Could not extract pid from line %s", line));
    }
    tracer_pid = potential_pid;
    break;
  }

  if (!tracer_pid.has_value()) {
    return ErrorMessage(
        absl::StrFormat("Could not find \"%s\" in %s", kTracerPidStr, status_file_name));
  }

  return tracer_pid.value();
}

}  // namespace orbit_base
