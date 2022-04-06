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

namespace {

ErrorMessageOr<Debugger::StartInfo> CreateStartInfoOrError(
    const ErrorMessageOr<ProcessInfo>& process_info_or_error) {
  if (process_info_or_error.has_error()) return process_info_or_error.error();
  const ProcessInfo& process_info = process_info_or_error.value();
  Debugger::StartInfo start_info;
  start_info.working_directory = process_info.working_directory;
  start_info.command_line = process_info.command_line;
  start_info.process_id = process_info.process_id;
  return start_info;
}

}  // namespace

Debugger::Debugger(std::vector<DebugEventListener*> debug_event_listeners)
    : debug_event_listeners_(std::move(debug_event_listeners)) {
  ORBIT_CHECK(debug_event_listeners_.size() != 0);
}

Debugger::~Debugger() { Wait(); }

ErrorMessageOr<Debugger::StartInfo> Debugger::Start(const std::filesystem::path& executable,
                                                    const std::filesystem::path& working_directory,
                                                    const std::string_view arguments) {
  // Launch process and start debugging loop on the same separate thread.
  thread_ = std::thread(&Debugger::DebuggerThread, this, executable, working_directory,
                        std::string(arguments));

  // Wait for "DebuggerThread" to create the process and return result of process creation.
  return start_info_or_error_promise_.GetFuture().Get();
}

void Debugger::Detach() { detach_requested_ = true; }

void Debugger::Wait() {
  if (thread_.joinable()) thread_.join();
}

void Debugger::DebuggerThread(std::filesystem::path executable,
                              std::filesystem::path working_directory, std::string arguments) {
  orbit_base::SetCurrentThreadName("OrbitDebugger");

  // Create process to debug. This needs to happen on the same thread as calls to WaitForDebugEvent.
  auto process_info_or_error = CreateProcessToDebug(executable, working_directory, arguments);

  // Notify parent thread that the process creation result is ready.
  start_info_or_error_promise_.SetResult(CreateStartInfoOrError(process_info_or_error));

  // Start debugging loop if process was created successfully.
  if (!process_info_or_error.has_error()) {
    DebuggingLoop(process_info_or_error.value().process_id);
  }
}

void Debugger::DebuggingLoop(uint32_t process_id) {
  DEBUG_EVENT debug_event = {0};
  constexpr uint32_t kWaitForDebugEventMs = 500;
  constexpr uint32_t kSemaphoreExpiredErrorCode = 121;

  while (true) {
    if (!WaitForDebugEvent(&debug_event, kWaitForDebugEventMs)) {
      if (::GetLastError() != kSemaphoreExpiredErrorCode) {
        ORBIT_ERROR("%s", orbit_base::GetLastErrorAsString("WaitForDebugEvent"));
        detach_requested_ = true;
      }

      if (!detach_requested_) continue;
      DebugActiveProcessStop(process_id);
      break;
    }

    uint32_t continue_status = DispatchDebugEvent(debug_event);
    ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status);
  }
}

uint32_t Debugger::DispatchDebugEvent(const DEBUG_EVENT& debug_event) {
  switch (debug_event.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnCreateProcessDebugEvent(debug_event);
      }
      break;
    case EXIT_PROCESS_DEBUG_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnExitProcessDebugEvent(debug_event);
      }
      // The debugged process has exited, call detach to exit debugging loop.
      Detach();
      break;
    case CREATE_THREAD_DEBUG_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnCreateThreadDebugEvent(debug_event);
      }
      break;
    case EXIT_THREAD_DEBUG_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnExitThreadDebugEvent(debug_event);
      }
      break;
    case LOAD_DLL_DEBUG_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnLoadDllDebugEvent(debug_event);
      }
      break;
    case UNLOAD_DLL_DEBUG_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnUnLoadDllDebugEvent(debug_event);
      }
      break;
    case OUTPUT_DEBUG_STRING_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnOutputStringDebugEvent(debug_event);
      }
      break;
    case RIP_EVENT:
      for (auto* listener : debug_event_listeners_) {
        listener->OnRipEvent(debug_event);
      }
      break;
    case EXCEPTION_DEBUG_EVENT: {
      if (debug_event.u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT) {
        for (auto* listener : debug_event_listeners_) {
          listener->OnBreakpointDebugEvent(debug_event);
        }
      } else {
        for (auto* listener : debug_event_listeners_) {
          listener->OnExceptionDebugEvent(debug_event);
        }
        return DBG_EXCEPTION_NOT_HANDLED;
      }
      break;
    }
    default:
      ORBIT_ERROR("Unhandled debugger event code: %u", debug_event.dwDebugEventCode);
  }

  return DBG_CONTINUE;
}

}  // namespace orbit_windows_utils
