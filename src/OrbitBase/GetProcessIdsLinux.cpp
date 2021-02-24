// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <filesystem>
#include <optional>
#include <system_error>

#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "absl/strings/numbers.h"

namespace orbit_base {

namespace fs = std::filesystem;

static std::optional<pid_t> ProcEntryToPid(const std::filesystem::directory_entry& entry) {
  std::error_code error;
  bool is_directory = entry.is_directory(error);
  if (error) {
    ERROR("Unable to stat \"%s\": %s", entry.path(), error.message());
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
    ERROR("Unable to ls /proc: %s", error.message());
    return {};
  }

  std::vector<pid_t> pids;

  for (auto it = fs::begin(proc), end = fs::end(proc); it != end; it.increment(error)) {
    if (error) {
      ERROR("directory_iterator::increment failed with: %s (stopping)", error.message());
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
    ERROR("Getting tids of threads of process %d: %s", pid, error.message());
    return {};
  }

  std::vector<pid_t> tids;
  for (auto it = fs::begin(proc_pid_task), end = fs::end(proc_pid_task); it != end;
       it.increment(error)) {
    if (error) {
      ERROR("directory_iterator::increment failed with: %s (stopping)", error.message());
      break;
    }
    if (auto tid = ProcEntryToPid(*it); tid.has_value()) {
      tids.emplace_back(tid.value());
    }
  }
  return tids;
}

}  // namespace orbit_base