// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_DEBUGGER_H_
#define WINDOWS_UTILS_DEBUGGER_H_

// clang-format off
#include <Windows.h>
#include <psapi.h>
// clang-format on

#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "WindowsUtils/CreateProcess.h"

namespace orbit_windows_utils {

// Base class for debug event listeners to be used by the "Debugger" class below.
struct DebugEventListener {
  virtual ~DebugEventListener() = default;

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
};

// "Debugger" allows to launch a process and receive debugging events such as process and thread
// creation and exit, module loads and unloads, breakpoints, etc. Debugging events are relayed to
// listeners of type "DebugEventListener" which need to be specified at creation.
class Debugger {
 public:
  explicit Debugger(std::vector<DebugEventListener*> debug_event_listeners);
  ~Debugger();

  struct StartInfo {
    std::string working_directory;
    std::string command_line;
    uint32_t process_id = 0;
  };

  // Start debugging, this call is non-blocking.
  ErrorMessageOr<StartInfo> Start(const std::filesystem::path& executable,
                                  const std::filesystem::path& working_directory,
                                  const std::string_view arguments);
  // Detach debugger from process.
  void Detach();
  // Wait for debuggee to exit.
  void Wait();

 private:
  void DebuggerThread(std::filesystem::path executable, std::filesystem::path working_directory,
                      std::string arguments);
  void DebuggingLoop(uint32_t process_id);
  uint32_t DispatchDebugEvent(const DEBUG_EVENT& event);

  std::thread thread_;
  std::atomic<bool> detach_requested_ = false;
  orbit_base::Promise<ErrorMessageOr<StartInfo>> start_info_or_error_promise_;
  std::vector<DebugEventListener*> debug_event_listeners_;
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_DEBUGGER_H_
