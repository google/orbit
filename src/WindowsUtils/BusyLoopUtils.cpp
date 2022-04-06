// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/BusyLoopUtils.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/ReadProcessMemory.h"
#include "WindowsUtils/SafeHandle.h"
#include "WindowsUtils/WriteProcessMemory.h"

namespace orbit_windows_utils {

namespace {

using orbit_base::GetLastErrorAsErrorMessage;

ErrorMessageOr<void> FlushInstructionCache(HANDLE process_handle, void* address, size_t size) {
  if (::FlushInstructionCache(process_handle, address, size) == 0) {
    return GetLastErrorAsErrorMessage("FlushInstructionCache");
  }
  return outcome::success();
}

}  // namespace

ErrorMessageOr<BusyLoopInfo> InstallBusyLoopAtAddress(HANDLE process_handle, void* address) {
  // The busy loop code is a 2 bytes long reverse short jump that jumps back 2 bytes onto itself,
  // i.e. "jmp (%rip)-2".
  constexpr const char kBusyLoopCode[] = {0xEB, 0xFE};

  BusyLoopInfo busy_loop;
  busy_loop.address = absl::bit_cast<uint64_t>(address);
  busy_loop.process_id = GetProcessId(process_handle);

  // Copy original bytes before installing busy loop.
  OUTCOME_TRY(busy_loop.original_bytes,
              ReadProcessMemory(busy_loop.process_id, busy_loop.address, sizeof(kBusyLoopCode)));

  // Install busy loop.
  OUTCOME_TRY(WriteProcessMemory(process_handle, address, kBusyLoopCode));

  // Flush instruction cache.
  OUTCOME_TRY(FlushInstructionCache(process_handle, address, sizeof(kBusyLoopCode)));

  return busy_loop;
}

ErrorMessageOr<void> RemoveBusyLoop(const BusyLoopInfo& busy_loop_info) {
  OUTCOME_TRY(SafeHandle process_handle,
              OpenProcess(PROCESS_ALL_ACCESS, /*inherit_handle=*/false, busy_loop_info.process_id));

  // Remove the busy loop and restore the original bytes.
  void* address = absl::bit_cast<void*>(busy_loop_info.address);
  const std::string& original_bytes = busy_loop_info.original_bytes;
  OUTCOME_TRY(WriteProcessMemory(*process_handle, address, original_bytes));

  // Flush instruction cache.
  OUTCOME_TRY(FlushInstructionCache(*process_handle, address, original_bytes.size()));

  return outcome::success();
}

ErrorMessageOr<void> SetThreadInstructionPointer(HANDLE thread_handle,
                                                 uint64_t instruction_pointer) {
  CONTEXT c = {0};
  c.ContextFlags = CONTEXT_CONTROL;
  if (!GetThreadContext(thread_handle, &c)) {
    return GetLastErrorAsErrorMessage("GetThreadContext");
  }

  c.Rip = instruction_pointer;

  if (SetThreadContext(thread_handle, &c) == 0) {
    return GetLastErrorAsErrorMessage("SetThreadContext");
  }

  return outcome::success();
}

ErrorMessageOr<void> SuspendThread(HANDLE thread_handle) {
  if (::SuspendThread(thread_handle) == (DWORD)-1) {
    return GetLastErrorAsErrorMessage("SuspendThread");
  }
  return outcome::success();
}

ErrorMessageOr<void> ResumeThread(HANDLE thread_handle) {
  if (::ResumeThread(thread_handle) == (DWORD)-1) {
    return GetLastErrorAsErrorMessage("ResumeThread");
  }
  return outcome::success();
}

}  // namespace orbit_windows_utils
