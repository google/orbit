// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessService/ProcessList.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "ProcessService/Process.h"
#include "ProcessServiceUtils.h"

namespace orbit_process_service_internal {

ErrorMessageOr<void> ProcessList::Refresh() {
  absl::flat_hash_map<pid_t, Process> updated_processes{};

  // TODO(b/161423785): This for loop should be refactored. For example, when
  //  parts are in a separate function, OUTCOME_TRY could be used to simplify
  //  error handling.
  std::error_code error;
  auto directory_iterator = std::filesystem::directory_iterator("/proc", error);
  if (error) {
    return ErrorMessage(absl::StrFormat("Unable to iterate /proc directory: %s", error.message()));
  }

  for (auto it = begin(directory_iterator); it != end(directory_iterator); it.increment(error)) {
    if (error) {
      return ErrorMessage(
          absl::StrFormat("Unable to iterate /proc directory: %s", error.message()));
    }

    bool is_directory = it->is_directory(error);
    if (error) {
      ORBIT_ERROR("Unable to stat \"%s\" directory entry: %s", it->path(), error.message());
      continue;
    }

    if (!is_directory) continue;

    const std::filesystem::path& path = it->path();
    std::string folder_name = path.filename().string();

    uint32_t pid;
    if (!absl::SimpleAtoi(folder_name, &pid)) continue;

    const auto iter = processes_.find(pid);

    if (iter != processes_.end()) {
      auto process = processes_.extract(iter);

      const auto total_cpu_time = orbit_process_service::GetCumulativeTotalCpuTime();
      const auto cpu_time = orbit_process_service::GetCumulativeCpuTimeFromProcess(process.key());
      if (cpu_time && total_cpu_time) {
        process.mapped().UpdateCpuUsage(cpu_time.value(), total_cpu_time.value());
      } else {
        // We don't fail in this case. This could be a permission problem which might occur when not
        // running as root.
        ORBIT_ERROR("Could not update the CPU usage of process %d", process.key());
      }

      updated_processes.insert(std::move(process));
      continue;
    }

    auto process_or_error = Process::FromPid(pid);

    if (process_or_error.has_error()) {
      // We don't fail in this case. This could be a permission problem which is restricted to a
      // small amount of processes.
      ORBIT_ERROR("Could not create process list entry for pid %u: %s", pid,
                  process_or_error.error().message());
      continue;
    }

    updated_processes.emplace(pid, std::move(process_or_error.value()));
  }

  processes_ = std::move(updated_processes);

  if (processes_.empty()) {
    return ErrorMessage{
        "Could not determine a single process from the proc-filesystem. Something seems to wrong."};
  }

  return outcome::success();
}

}  // namespace orbit_process_service_internal
