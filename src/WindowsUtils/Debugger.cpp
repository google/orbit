// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/Debugger.h"

#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/StringConversion.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/CreateProcess.h"
#include "WindowsUtils/SafeHandle.h"

namespace orbit_windows_utils {

Debugger::Debugger() : process_info_or_error_(ErrorMessage()) {}
Debugger::~Debugger() { Wait(); }

ErrorMessageOr<ProcessInfo> Debugger::Start(const std::filesystem::path& executable,
                                            const std::filesystem::path& working_directory,
                                            std::string_view arguments) {
  // Launch process and start debugging loop on the same separate thread.
  thread_ = std::thread(&Debugger::DebuggerThread, this, executable, working_directory,
                        std::string(arguments));

  // We use "create_process_promise_" to wait until the result of "CreateProcessToDebug", which is
  // stored in "process_info_or_error_", is available from the  debugger thread.
  create_process_promise_.GetFuture().Wait();
  return std::move(process_info_or_error_);
}

void Debugger::Detach() { detach_requested_ = true; }

void Debugger::Wait() {
  if (thread_.joinable()) thread_.join();
}

void Debugger::DebuggerThread(std::filesystem::path executable,
                              std::filesystem::path working_directory, std::string arguments) {
  orbit_base::SetCurrentThreadName("OrbitDebugger");

  // Only the thread that created the process being debugged can call WaitForDebugEvent.
  process_info_or_error_ = CreateProcessToDebug(executable, working_directory, arguments);
  create_process_promise_.SetResult(true);
  if (process_info_or_error_.has_error()) {
    return;
  }

  DEBUG_EVENT debug_event = {0};
  bool continue_debugging = true;
  DWORD continue_status = DBG_CONTINUE;
  constexpr uint32_t kWaitForDebugEventMs = 500;
  constexpr uint32_t kSemaphoreExpiredErrorCode = 121;

  while (continue_debugging) {
    if (!WaitForDebugEvent(&debug_event, kWaitForDebugEventMs)) {
      if (::GetLastError() != kSemaphoreExpiredErrorCode) {
        ORBIT_ERROR("Calling \"WaitForDebugEvent\": %s", orbit_base::GetLastErrorAsString());
        detach_requested_ = true;
      }
      if (!detach_requested_) continue;
      DebugActiveProcessStop(process_info_or_error_.value().process_id);
      break;
    }

    switch (debug_event.dwDebugEventCode) {
      case CREATE_PROCESS_DEBUG_EVENT:
        OnCreateProcessDebugEvent(debug_event);
        break;
      case EXIT_PROCESS_DEBUG_EVENT:
        OnExitProcessDebugEvent(debug_event);
        Detach();
        break;
      case CREATE_THREAD_DEBUG_EVENT:
        OnCreateThreadDebugEvent(debug_event);
        break;
      case EXIT_THREAD_DEBUG_EVENT:
        OnExitThreadDebugEvent(debug_event);
        break;
      case LOAD_DLL_DEBUG_EVENT:
        OnLoadDllDebugEvent(debug_event);
        break;
      case UNLOAD_DLL_DEBUG_EVENT:
        OnUnLoadDllDebugEvent(debug_event);
        break;
      case OUTPUT_DEBUG_STRING_EVENT:
        OnOutputStringDebugEvent(debug_event);
        break;
      case RIP_EVENT:
        OnRipEvent(debug_event);
        break;
      case EXCEPTION_DEBUG_EVENT: {
        EXCEPTION_DEBUG_INFO& exception = debug_event.u.Exception;
        if (exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT) {
          OnBreakpointDebugEvent(debug_event);
        } else {
          OnExceptionDebugEvent(debug_event);
          if (exception.dwFirstChance == 1) {
            ORBIT_LOG("First chance exception at %x, exception-code: 0x%08x",
                      exception.ExceptionRecord.ExceptionAddress,
                      exception.ExceptionRecord.ExceptionCode);
          }
          continue_status = DBG_EXCEPTION_NOT_HANDLED;
        }
        break;
      }
      default:
        ORBIT_ERROR("Unhandled debugger event code: %u", debug_event.dwDebugEventCode);
    }

    ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status);
    continue_status = DBG_CONTINUE;
  }
}

}  // namespace orbit_windows_utils
