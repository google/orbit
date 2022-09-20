// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_PROCESS_LAUNCHER_H_
#define WINDOWS_UTILS_PROCESS_LAUNCHER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include "WindowsUtils/BusyLoopLauncher.h"

namespace orbit_windows_utils {

// Class used to launch a process, optionally pausing it at the entry point and maintaining
// information required to suspend and resume the process' main thread. A "paused" process is
// initially busy-looping at entry point. To remove the busy loop but remain paused at entry,
// we call "SuspendProcess". "ResumeProcess" can then be called to resume normal execution.
class ProcessLauncher {
 public:
  ErrorMessageOr<uint32_t> LaunchProcess(const std::filesystem::path& executable,
                                         const std::filesystem::path& working_directory,
                                         const std::string_view arguments,
                                         bool pause_at_entry_point);

  // Suspend a process that is currently spinning ("paused") at entry point and replace the busy
  // loop by the original instructions. This call will fail if the process can't be found or if it
  // is not in the spinning state.
  ErrorMessageOr<void> SuspendProcessSpinningAtEntryPoint(uint32_t process_id);

  // Resume a process that was suspended using the "SuspendProcess" method above. This call will
  // fail if the process can't be found or if it is not in the suspended state.
  ErrorMessageOr<void> ResumeProcessSuspendedAtEntryPoint(uint32_t process_id);

 private:
  ErrorMessageOr<uint32_t> LaunchProcess(const std::filesystem::path& executable,
                                         const std::filesystem::path& working_directory,
                                         const std::string_view arguments);

  ErrorMessageOr<uint32_t> LaunchProcessPausedAtEntryPoint(
      const std::filesystem::path& executable, const std::filesystem::path& working_directory,
      const std::string_view arguments);

  absl::Mutex mutex_;
  absl::flat_hash_map<uint32_t, std::unique_ptr<orbit_windows_utils::BusyLoopLauncher>>
      busy_loop_launchers_by_pid_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_PROCESS_LAUNCHER_H_
