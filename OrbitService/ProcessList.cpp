// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessList.h"

#include <absl/strings/ascii.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>

#include <filesystem>
#include <unordered_map>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "Utils.h"

namespace orbit_service {

ErrorMessageOr<void> ProcessList::Refresh() {
  auto cpu_result = utils::GetCpuUtilization();
  if (!cpu_result) {
    return outcome::failure(absl::StrFormat("Unable to retrieve cpu usage of processes: %s",
                                            cpu_result.error().message()));
  }
  std::unordered_map<int32_t, double> cpu_usage_map = std::move(cpu_result.value());

  std::vector<Process> updated_processes;

  // TODO(b/161423785): This for loop should be refactored. For example, when
  //  parts are in a separate function, OUTCOME_TRY could be used to simplify
  //  error handling. Also use ErrorMessageOr
  for (const auto& directory_entry : std::filesystem::directory_iterator("/proc")) {
    if (!directory_entry.is_directory()) continue;

    const std::filesystem::path& path = directory_entry.path();
    std::string folder_name = path.filename().string();

    uint32_t pid;
    if (!absl::SimpleAtoi(folder_name, &pid)) continue;

    auto iter = processes_map_.find(pid);
    if (iter != processes_map_.end()) {
      Process& process = *(iter->second);
      process.set_cpu_usage(cpu_usage_map[process.pid()]);
      updated_processes.push_back(process);
      continue;
    }

    auto process = Process::FromPidAndCpuUsage(pid, cpu_usage_map[pid]);
    if (process) {
      updated_processes.emplace_back(std::move(process.value()));
    } else {
      // We don't fail in this case. This could be a permission problem which is restricted to a
      // small amount of processes.
      ERROR("Could not create process list entry for pid %d: %s", pid, process.error().message());
    }
  }

  processes_ = std::move(updated_processes);
  processes_map_.clear();
  for (auto& process : processes_) {
    processes_map_[process.pid()] = &process;
  }

  return outcome::success();
}

}  // namespace orbit_service
