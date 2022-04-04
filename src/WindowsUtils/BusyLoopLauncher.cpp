// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/BusyLoopLauncher.h"

namespace orbit_windows_utils {

BusyLoopLauncher::BusyLoopLauncher() : debugger_({this}) {}

ErrorMessageOr<BusyLoopInfo> BusyLoopLauncher::StartWithBusyLoopAtEntryPoint(
    const std::filesystem::path& executable, const std::filesystem::path& working_directory,
    const std::string_view arguments) {
  // Launch process as debuggee in order to receive debugging events.
  OUTCOME_TRY(debugger_.Start(executable, working_directory, arguments));

  // Wait for "OnCreateProcessDebugEvent" to be called on process creation and for the result of the
  // busy loop installation to be ready.
  return busy_loop_info_or_error_promise_.GetFuture().Get();
}

void BusyLoopLauncher::OnCreateProcessDebugEvent(const DEBUG_EVENT& event) {
  // Try installing a busy loop at entry point.
  busy_loop_info_or_error_promise_.SetResult(InstallBusyLoopAtAddress(
      event.u.CreateProcessInfo.hProcess, event.u.CreateProcessInfo.lpStartAddress));

  // At this point, we don't need to continue debugging the process, detach.
  debugger_.Detach();
}

}  // namespace orbit_windows_utils
