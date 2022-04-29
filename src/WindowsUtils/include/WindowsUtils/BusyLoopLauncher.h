// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_BUSY_LOOP_LAUNCHER_H_
#define WINDOWS_UTILS_BUSY_LOOP_LAUNCHER_H_

#include "OrbitBase/Result.h"
#include "WindowsUtils/BusyLoopUtils.h"
#include "WindowsUtils/Debugger.h"

namespace orbit_windows_utils {

// Utility to launch a process and install a busy loop at entry point. This is mainly used to allow
// dll injection as early as possible during process creation. The class is single-use only, it can
// not be reused to launch multiple processes.
//
// Typical usage:
//  1. Call "StartWithBusyLoopAtEntryPoint" to launch process that will spin at entry point.
//  2. Inject dll.
//  3. Call "SuspendMainThreadAndRemoveBusyLoop" to avoid unnecessary resource hog.
//  4. Call "ResumeMainThread" when ready to start process execution.
//
// This class is not thread-safe.
class BusyLoopLauncher : public DebugEventListener {
 public:
  BusyLoopLauncher();

  ErrorMessageOr<BusyLoopInfo> StartWithBusyLoopAtEntryPoint(
      const std::filesystem::path& executable, const std::filesystem::path& working_directory,
      const std::string_view arguments);
  ErrorMessageOr<void> SuspendMainThreadAndRemoveBusyLoop();
  ErrorMessageOr<void> ResumeMainThread();

  bool IsProcessSuspended() const { return state_ == State::kMainThreadSuspended; }
  void WaitForProcessToExit();

 protected:
  void OnCreateProcessDebugEvent(const DEBUG_EVENT& event) override;

  // Unused DebugEventListener methods.
  void OnExitProcessDebugEvent(const DEBUG_EVENT& event) override{};
  void OnCreateThreadDebugEvent(const DEBUG_EVENT& event) override{};
  void OnExitThreadDebugEvent(const DEBUG_EVENT& event) override{};
  void OnLoadDllDebugEvent(const DEBUG_EVENT& event) override{};
  void OnUnLoadDllDebugEvent(const DEBUG_EVENT& event) override{};
  void OnBreakpointDebugEvent(const DEBUG_EVENT& event) override{};
  void OnOutputStringDebugEvent(const DEBUG_EVENT& event) override{};
  void OnExceptionDebugEvent(const DEBUG_EVENT& event) override{};
  void OnRipEvent(const DEBUG_EVENT& event) override{};

 private:
  enum class State {
    kInitialState,
    kMainThreadInBusyLoop,
    kMainThreadSuspended,
    kMainThreadResumed,
    kProcessExited
  };

  Debugger debugger_;
  orbit_base::Promise<ErrorMessageOr<BusyLoopInfo>> busy_loop_info_or_error_promise_;
  HANDLE main_thread_handle_ = nullptr;
  State state_ = State::kInitialState;
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_BUSY_LOOP_LAUNCHER_H_
