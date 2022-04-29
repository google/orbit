// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_PROCESS_LAUNCHER_H_
#define WINDOWS_UTILS_PROCESS_LAUNCHER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include "WindowsUtils/BusyLoopLauncher.h"

namespace orbit_windows_utils {

// Class used to launch a process, optionally pausing it at the entry point and
// maintaining information required to suspend and resume the process main thread.
class ProcessLauncher {
 public:
  ErrorMessageOr<uint32_t> LaunchProcess(const std::filesystem::path& executable,
                                         const std::filesystem::path& working_directory,
                                         const std::string_view arguments,
                                         bool pause_at_entry_point);

  ErrorMessageOr<void> SuspendProcess(uint32_t process_id);
  ErrorMessageOr<void> ResumeProcess(uint32_t process_id);

 private:
  ErrorMessageOr<uint32_t> LaunchProcess(const std::filesystem::path& executable,
                                         const std::filesystem::path& working_directory,
                                         const std::string_view arguments);

  ErrorMessageOr<uint32_t> LaunchProcessPausedAtEntryPoint(
      const std::filesystem::path& executable, const std::filesystem::path& working_directory,
      const std::string_view arguments);

  absl::Mutex mutex_;
  absl::flat_hash_map<uint32_t, std::unique_ptr<orbit_windows_utils::BusyLoopLauncher>>
      busy_loop_launchers_by_pid_ GUARDED_BY(mutex_);
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_PROCESS_LAUNCHER_H_
