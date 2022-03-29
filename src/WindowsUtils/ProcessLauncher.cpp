// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ProcessLauncher.h"

#include <absl/base/casts.h>

#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/ReadProcessMemory.h"
#include "WindowsUtils/SafeHandle.h"
#include "WindowsUtils/WriteProcessMemory.h"

//clang-format off
#include <Windows.h>
#include <processthreadsapi.h>
//clang-format on

namespace orbit_windows_utils {

namespace {

ErrorMessageOr<BusyLoopInfo> InstallBusyLoopAtAddress(HANDLE process_handle, HANDLE thread_handle,
                                                      void* address) {
  constexpr const char kBusyLoopCode[] = {0xEB, 0xFE};
  BusyLoopInfo busy_loop;
  busy_loop.address = absl::bit_cast<uint64_t>(address);
  busy_loop.process_id = GetThreadId(process_handle);
  busy_loop.thread_id = GetThreadId(thread_handle);

  // Copy original bytes before installing busy loop.
  OUTCOME_TRY(
      busy_loop.original_bytes,
      ReadProcessMemory(GetProcessId(process_handle), busy_loop.address, sizeof(kBusyLoopCode)));

  // Install busy loop.
  OUTCOME_TRY(WriteProcessMemory(process_handle, address, kBusyLoopCode));

  // Flush instruction cache.
  if (FlushInstructionCache(process_handle, address, sizeof(kBusyLoopCode)) == 0) {
    return orbit_base::GetLastError("Calling \"FlushInstructionCache\"");
  }

  return busy_loop;
}

ErrorMessageOr<void> SetThreadInstructionPointer(HANDLE thread_handle,
                                                 uint64_t instruction_pointer) {
  CONTEXT c = {0};
  c.ContextFlags = CONTEXT_CONTROL;
  if (!GetThreadContext(thread_handle, &c)) {
    return orbit_base::GetLastError("Calling \"GetThreadContext\"");
  }

  c.Rip = instruction_pointer;

  if (SetThreadContext(thread_handle, &c) == 0) {
    return orbit_base::GetLastError("Calling \"SetThreadContext\"");
  }

  return outcome::success();
}

ErrorMessageOr<void> WriteInstructionsAtAddress(const std::string& byte_buffer, void* address) {
  // Mark memory page for writing.
  DWORD old_protect = 0;
  if (!VirtualProtect(address, byte_buffer.size(), PAGE_EXECUTE_READWRITE, &old_protect)) {
    return orbit_base::GetLastError("Calling \"VirtualProtect\"");
  }

  // Write original bytes.
  memcpy(address, byte_buffer.data(), byte_buffer.size());

  // Restore original page protection.
  if (!VirtualProtect(address, byte_buffer.size(), old_protect, &old_protect)) {
    return orbit_base::GetLastError("Calling \"VirtualProtect\"");
  }

  FlushInstructionCache(GetCurrentProcess(), address, byte_buffer.size());
  return outcome::success();
}

// From within the target process. Restore original bytes in the main thread that is busy-looping at
// the entry point.
ErrorMessageOr<void> SuspendBusyLoopThread(const BusyLoopInfo& busy_loop_info) {
  // Get the busy-loop thread handle.
  SafeHandle thread_handle(OpenThread(THREAD_ALL_ACCESS, FALSE, busy_loop_info.thread_id));
  if (*thread_handle == nullptr) {
    return orbit_base::GetLastError("Calling \"OpenThread\"");
  }

  // Suspend the busy-loop thread.
  if (SuspendThread(*thread_handle) == (DWORD)-1) {
    return orbit_base::GetLastError("Calling \"SuspendThread\"");
  }

  // Remove the busy loop and restore the original bytes.
  void* busy_loop_address = absl::bit_cast<void*>(busy_loop_info.address);
  OUTCOME_TRY(WriteInstructionsAtAddress(busy_loop_info.original_bytes, busy_loop_address));

  // Make sure the instruction pointer is back to the entry point.
  OUTCOME_TRY(SetThreadInstructionPointer(*thread_handle, busy_loop_info.address));

  return outcome::success();
}

ErrorMessageOr<void> ResumeThread(uint32_t thread_id) {
  SafeHandle thread_handle(OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id));
  if (*thread_handle == nullptr) {
    return orbit_base::GetLastError("Calling \"OpenThread\"");
  }

  if (::ResumeThread(*thread_handle) == (DWORD)-1) {
    return orbit_base::GetLastError("Calling \"OpenThread\"");
  }

  return outcome::success();
}

}  // namespace

ErrorMessageOr<BusyLoopInfo> ProcessLauncher::StartWithBusyLoopAtEntryPoint(
    std::filesystem::path executable, std::filesystem::path working_directory,
    std::string_view arguments) {
  install_busy_loop_ = true;
  OUTCOME_TRY(ProcessInfo process_info, Debugger::Start(executable, working_directory, arguments));

  // Wait until the busy loop has been installed in "OnCreateProcessDebugEvent".
  busy_loop_ready_promise_.GetFuture().Wait();
  if (busy_loop_info_or_error_.has_error()) {
    return busy_loop_info_or_error_.error();
  }

  return busy_loop_info_or_error_;
}

void ProcessLauncher::OnCreateProcessDebugEvent(const DEBUG_EVENT& event) {
  if (!install_busy_loop_) return;
  // At this point, the debuggee is suspended at the entry point. We want to install a busy loop
  // so that the process can resume executing, albeit spinning at the entry point, which will allow
  // us to inject a dll.
  busy_loop_info_or_error_ = InstallBusyLoopAtAddress(event.u.CreateProcessInfo.hProcess,
                                                      event.u.CreateProcessInfo.hThread,
                                                      event.u.CreateProcessInfo.lpStartAddress);
  busy_loop_ready_promise_.SetResult(true);
}

}  // namespace orbit_windows_utils
