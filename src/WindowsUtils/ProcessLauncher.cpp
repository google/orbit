// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ProcessLauncher.h"

#include "WindowsUtils/CreateProcess.h"

namespace orbit_windows_utils {

using orbit_windows_utils::BusyLoopInfo;
using orbit_windows_utils::BusyLoopLauncher;
using orbit_windows_utils::ProcessInfo;

ErrorMessageOr<uint32_t> ProcessLauncher::LaunchProcess(
    const std::filesystem::path& executable, const std::filesystem::path& working_directory,
    const std::string_view arguments, bool pause_at_entry_point) {
  if (pause_at_entry_point) {
    return LaunchProcessPausedAtEntryPoint(executable, working_directory, arguments);
  } else {
    return LaunchProcess(executable, working_directory, arguments);
  }
}

ErrorMessageOr<uint32_t> ProcessLauncher::LaunchProcess(
    const std::filesystem::path& executable, const std::filesystem::path& working_directory,
    const std::string_view arguments) {
  OUTCOME_TRY(ProcessInfo & process_info,
              orbit_windows_utils::CreateProcess(executable, working_directory, arguments));
  return process_info.process_id;
}

ErrorMessageOr<uint32_t> ProcessLauncher::LaunchProcessPausedAtEntryPoint(
    const std::filesystem::path& executable, const std::filesystem::path& working_directory,
    const std::string_view arguments) {
  std::unique_ptr<BusyLoopLauncher> launcher = std::make_unique<BusyLoopLauncher>();
  OUTCOME_TRY(BusyLoopInfo busy_loop_info,
              launcher->StartWithBusyLoopAtEntryPoint(executable, working_directory, arguments));
  uint32_t pid = busy_loop_info.process_id;
  absl::MutexLock lock(&mutex_);
  ORBIT_CHECK(busy_loop_launchers_by_pid_.count(pid) == 0);
  busy_loop_launchers_by_pid_.emplace(pid, std::move(launcher));
  return pid;
}

ErrorMessageOr<void> ProcessLauncher::SuspendProcessSpinningAtEntryPoint(uint32_t process_id) {
  absl::MutexLock lock(&mutex_);
  auto it = busy_loop_launchers_by_pid_.find(process_id);
  if (it == busy_loop_launchers_by_pid_.end()) {
    return ErrorMessage("Trying to suspend unknown process");
  }
  BusyLoopLauncher* launcher = it->second.get();
  OUTCOME_TRY(launcher->SuspendMainThreadAndRemoveBusyLoop());
  return outcome::success();
}

ErrorMessageOr<void> ProcessLauncher::ResumeProcessSuspendedAtEntryPoint(uint32_t process_id) {
  absl::MutexLock lock(&mutex_);
  auto it = busy_loop_launchers_by_pid_.find(process_id);
  if (it == busy_loop_launchers_by_pid_.end()) {
    return ErrorMessage("Trying to resume unknown process");
  }
  BusyLoopLauncher* launcher = it->second.get();
  OUTCOME_TRY(launcher->ResumeMainThread());
  busy_loop_launchers_by_pid_.erase(process_id);
  return outcome::success();
}

}  // namespace orbit_windows_utils