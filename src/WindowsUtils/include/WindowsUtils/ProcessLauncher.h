// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_PROCESS_LAUNCHER_H_
#define WINDOWS_UTILS_PROCESS_LAUNCHER_H_

#include "WindowsUtils/Debugger.h"

namespace orbit_windows_utils {

struct BusyLoopInfo {
  uint32_t process_id = 0;
  uint32_t thread_id = 0;
  uint64_t address = 0;
  std::string original_bytes;
};

// Profiling from entry point, sequence of events:
//
// - Orbit starts a process and becomes its debugger.
// - Orbit installs a busy loop at the entry point on the "OnCreateProcessDebugEvent" call
//   to prevent process from continuing execution passed the entry point.
// - Orbit continues execution of the process which will spin on the busy loop.
// - Orbit injects dll into target process, for this to work, the process needs to be running, hence
//   the busy-loop.
// - Orbit sends message to target with busy-loop information so that we can suspend the thread
//   instead of busy-looping. The "Suspend" call can only happen from within the target process.
// - Target process suspends the busy loop thread and replaces the busy-loop by the original
//   instruction bytes.
// - On capture start, Orbit sends the target process a message to resume the

class ProcessLauncher : public Debugger {
  ErrorMessageOr<BusyLoopInfo> StartWithBusyLoopAtEntryPoint(
      std::filesystem::path executable, std::filesystem::path working_directory,
      std::string_view arguments);

 protected:
  void OnCreateProcessDebugEvent(const DEBUG_EVENT& event) override;

  // Intentionally not implemented.
  void OnExitProcessDebugEvent(const DEBUG_EVENT& event) override {}
  void OnCreateThreadDebugEvent(const DEBUG_EVENT& event) override {}
  void OnExitThreadDebugEvent(const DEBUG_EVENT& event) override {}
  void OnLoadDllDebugEvent(const DEBUG_EVENT& event) override {}
  void OnUnLoadDllDebugEvent(const DEBUG_EVENT& event) override {}
  void OnBreakpointDebugEvent(const DEBUG_EVENT& event) override{};
  void OnOutputStringDebugEvent(const DEBUG_EVENT& event) override {}
  void OnExceptionDebugEvent(const DEBUG_EVENT& event) override {}
  void OnRipEvent(const DEBUG_EVENT& event) override {}

 private:
  bool install_busy_loop_ = false;
  orbit_base::Promise<bool> busy_loop_ready_promise_;
  ErrorMessageOr<BusyLoopInfo> busy_loop_info_or_error_;
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_PROCESS_LAUNCHER_H_
