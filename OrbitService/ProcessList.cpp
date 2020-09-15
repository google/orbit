// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessList.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>

#include <filesystem>
#include <unordered_map>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "Utils.h"

namespace orbit_service {

ErrorMessageOr<void> ProcessList::Refresh() {
  absl::flat_hash_map<pid_t, Process> updated_processes{};

  // TODO(b/161423785): This for loop should be refactored. For example, when
  //  parts are in a separate function, OUTCOME_TRY could be used to simplify
  //  error handling. Also use ErrorMessageOr
  for (const auto& directory_entry : std::filesystem::directory_iterator("/proc")) {
    if (!directory_entry.is_directory()) continue;

    const std::filesystem::path& path = directory_entry.path();
    std::string folder_name = path.filename().string();

    uint32_t pid;
    if (!absl::SimpleAtoi(folder_name, &pid)) continue;

    const auto iter = processes_.find(pid);
    if (iter != processes_.end()) {
      auto process = processes_.extract(iter);

      const auto total_cpu_time = utils::GetCumulativeTotalCpuTime();
      const auto cpu_time = utils::GetCumulativeCpuTimeFromProcess(process.key());
      if (cpu_time && total_cpu_time) {
        process.mapped().UpdateCpuUsage(cpu_time.value(), total_cpu_time.value());
      } else {
        // We don't fail in this case. This could be a permission problem which might occur when not
        // running as root.
        ERROR("Could not update the CPU usage of process %d", process.key());
      }

      updated_processes.insert(std::move(process));
      continue;
    }

    auto process = Process::FromPid(pid);

    if (process) {
      updated_processes.emplace(pid, std::move(process.value()));
    } else {
      // We don't fail in this case. This could be a permission problem which is restricted to a
      // small amount of processes.
      ERROR("Could not create process list entry for pid %d: %s", pid, process.error().message());
    }
  }

  processes_ = std::move(updated_processes);

  if (processes_.empty()) {
    return ErrorMessage{
        "Could not determine a single process from the proc-filesystem. Something seems to wrong."};
  }

  return outcome::success();
}

}  // namespace orbit_service
