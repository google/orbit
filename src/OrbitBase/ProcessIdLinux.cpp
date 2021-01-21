// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <filesystem>
#include <optional>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ProcessId.h"
#include "OrbitBase/Result.h"
#include "absl/strings/numbers.h"

namespace orbit_base {

namespace fs = std::filesystem;

namespace {

std::optional<pid_t> ProcEntryToPid(const std::filesystem::directory_entry& entry) {
  if (!entry.is_directory()) {
    return std::nullopt;
  }

  int potential_pid;
  if (!absl::SimpleAtoi(entry.path().filename().string(), &potential_pid)) {
    return std::nullopt;
  }

  if (potential_pid <= 0) {
    return std::nullopt;
  }

  return static_cast<pid_t>(potential_pid);
}

}  // namespace

std::vector<pid_t> GetAllPids() {
  fs::directory_iterator proc{"/proc"};
  std::vector<pid_t> pids;

  for (const auto& entry : proc) {
    if (auto pid = ProcEntryToPid(entry); pid.has_value()) {
      pids.emplace_back(pid.value());
    }
  }

  return pids;
}

std::vector<pid_t> GetTidsOfProcess(pid_t pid) {
  std::error_code error_code;
  fs::directory_iterator proc_pid_task{fs::path{"/proc"} / std::to_string(pid) / "task",
                                       error_code};
  if (error_code) {
    // The process with id `pid` could have stopped existing.
    ERROR("Getting tids of threads of process %d: %s", pid, error_code.message());
    return {};
  }

  std::vector<pid_t> tids;
  for (const auto& entry : proc_pid_task) {
    if (auto tid = ProcEntryToPid(entry); tid.has_value()) {
      tids.emplace_back(tid.value());
    }
  }
  return tids;
}

std::vector<pid_t> GetAllTids() {
  std::vector<pid_t> all_tids;
  for (pid_t pid : GetAllPids()) {
    std::vector<pid_t> process_tids = GetTidsOfProcess(pid);
    all_tids.insert(all_tids.end(), process_tids.begin(), process_tids.end());
  }
  return all_tids;
}

}  // namespace orbit_base