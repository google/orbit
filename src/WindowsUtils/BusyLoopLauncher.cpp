// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/BusyLoopLauncher.h"

#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"

namespace orbit_windows_utils {

BusyLoopLauncher::BusyLoopLauncher() : debugger_({this}) {}

ErrorMessageOr<BusyLoopInfo> BusyLoopLauncher::StartWithBusyLoopAtEntryPoint(
    const std::filesystem::path& executable, const std::filesystem::path& working_directory,
    const std::string_view arguments) {
  // Calling "StartWithBusyLoopAtEntryPoint" multiple times is not supported.
  ORBIT_CHECK(state_ == State::kInitialState);

  // Launch process as debuggee in order to receive debugging events.
  OUTCOME_TRY(debugger_.Start(executable, working_directory, arguments));
  state_ = State::kMainThreadInBusyLoop;

  // Wait for "OnCreateProcessDebugEvent" to be called on process creation and for the result of the
  // busy loop installation to be ready.
  return busy_loop_info_or_error_promise_.GetFuture().Get();
}

void BusyLoopLauncher::OnCreateProcessDebugEvent(const DEBUG_EVENT& event) {
  // Try installing a busy loop at entry point.
  ErrorMessageOr<BusyLoopInfo> busy_loop_info_or = InstallBusyLoopAtAddress(
      event.u.CreateProcessInfo.hProcess, event.u.CreateProcessInfo.lpStartAddress);

  // Set result on promise to notify parent thread.
  busy_loop_info_or_error_promise_.SetResult(busy_loop_info_or);
  if (busy_loop_info_or.has_error()) {
    return;
  }

  // Keep a handle on the main thread of the created process.
  main_thread_handle_ = event.u.CreateProcessInfo.hThread;
}

ErrorMessageOr<void> BusyLoopLauncher::SuspendMainThreadAndRemoveBusyLoop() {
  ORBIT_CHECK(main_thread_handle_ != nullptr);
  ORBIT_CHECK(state_ == State::kMainThreadInBusyLoop);

  // At this point, the future is already finished, there is no waiting.
  ORBIT_CHECK(busy_loop_info_or_error_promise_.GetFuture().IsFinished());
  OUTCOME_TRY(const BusyLoopInfo& busy_loop_info,
              busy_loop_info_or_error_promise_.GetFuture().Get());

  // Suspend the main thread.
  OUTCOME_TRY(SuspendThread(main_thread_handle_));
  state_ = State::kMainThreadSuspended;

  // Replace busy loop by original instructions.
  OUTCOME_TRY(RemoveBusyLoop(busy_loop_info));

  // Make sure the instruction pointer is set back to the entry point.
  OUTCOME_TRY(SetThreadInstructionPointer(main_thread_handle_, busy_loop_info.address));

  return outcome::success();
}

ErrorMessageOr<void> BusyLoopLauncher::ResumeMainThread() {
  ORBIT_CHECK(state_ == State::kMainThreadSuspended);
  ORBIT_CHECK(main_thread_handle_ != nullptr);
  OUTCOME_TRY(ResumeThread(main_thread_handle_));
  state_ = State::kMainThreadResumed;
  return outcome::success();
}

void BusyLoopLauncher::WaitForProcessToExit() {
  debugger_.Wait();
  state_ = State::kProcessExited;
}

}  // namespace orbit_windows_utils
