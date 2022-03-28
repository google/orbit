// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_DEBUGGER_H_
#define WINDOWS_UTILS_DEBUGGER_H_

// clang-format off
#include <Windows.h>
#include <psapi.h>
// clang-format on

#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "WindowsUtils/CreateProcess.h"

#include <filesystem>
#include <string>
#include <thread>

namespace orbit_windows_utils {

class Debugger {
 public:
  Debugger();
  virtual ~Debugger();

  // Start debugging, this call is non-blocking.
  ErrorMessageOr<ProcessInfo> Start(const std::filesystem::path& executable,
                                    const std::filesystem::path& working_directory,
                                    const std::string_view arguments);
  // Detach debugger from process.
  void Detach();
  // Wait for debuggee to exit.
  void Wait();

 protected:
  // The functions below will not be called from the thread "Start" was called from.
  virtual void OnCreateProcessDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnExitProcessDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnCreateThreadDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnExitThreadDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnLoadDllDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnUnLoadDllDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnBreakpointDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnOutputStringDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnExceptionDebugEvent(const DEBUG_EVENT& event) = 0;
  virtual void OnRipEvent(const DEBUG_EVENT& event) = 0;

 private:
  void DebuggerThread(std::filesystem::path executable, std::filesystem::path working_directory,
                      std::string arguments);
  std::thread thread_;
  std::atomic<bool> detach_requested_ = false;
  ErrorMessageOr<ProcessInfo> process_info_or_error_;
  orbit_base::Promise<bool> create_process_promise_;
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_DEBUGGER_H_
