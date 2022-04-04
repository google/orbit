// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_BUSY_LOOP_LAUNCHER_H_
#define WINDOWS_UTILS_BUSY_LOOP_LAUNCHER_H_

#include "WindowsUtils/BusyLoopUtils.h"
#include "WindowsUtils/Debugger.h"

namespace orbit_windows_utils {

// "BusyLoopLauncher" is a utility to launch a process and install a busy loop at entry point.
class BusyLoopLauncher : public DebugEventListener {
 public:
  BusyLoopLauncher();

  ErrorMessageOr<BusyLoopInfo> StartWithBusyLoopAtEntryPoint(
      const std::filesystem::path& executable, const std::filesystem::path& working_directory,
      const std::string_view arguments);

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
  Debugger debugger_;
  orbit_base::Promise<ErrorMessageOr<BusyLoopInfo>> busy_loop_info_or_error_promise_;
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_BUSY_LOOP_LAUNCHER_H_
