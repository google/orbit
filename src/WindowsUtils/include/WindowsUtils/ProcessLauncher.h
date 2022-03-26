// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_PROCESS_LAUNCHER_H_
#define WINDOWS_UTILS_PROCESS_LAUNCHER_H_

#include "WindowsUtils/Debugger.h"

namespace orbit_windows_utils {

struct BusyLoopInfo {
  uint32_t thread_id;
  uint64_t address;
  std::string original_bytes;
};

class ProcessLauncher : public Debugger {
 protected:
  // Injection takes place at entry point.
  void OnCreateProcessDebugEvent(const DEBUG_EVENT& event) override;
  void OnBreakpointDebugEvent(const DEBUG_EVENT& event) override;

  void OnExitProcessDebugEvent(const DEBUG_EVENT& event) override {}
  void OnCreateThreadDebugEvent(const DEBUG_EVENT& event) override {}
  void OnExitThreadDebugEvent(const DEBUG_EVENT& event) override {}
  void OnLoadDllDebugEvent(const DEBUG_EVENT& event) override {}
  void OnUnLoadDllDebugEvent(const DEBUG_EVENT& event) override {}
  void OnOutputStringDebugEvent(const DEBUG_EVENT& event) override {}
  void OnExceptionDebugEvent(const DEBUG_EVENT& event) override {}
  void OnRipEvent(const DEBUG_EVENT& event) override {}

 private:
  uint64_t start_address_ = 0;
  HANDLE process_handle_ = nullptr;
  uint32_t process_id_ = 0;
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_PROCESS_LAUNCHER_H_
