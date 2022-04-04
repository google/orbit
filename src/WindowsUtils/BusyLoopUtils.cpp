// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/BusyLoopUtils.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"
#include "WindowsUtils/ReadProcessMemory.h"
#include "WindowsUtils/SafeHandle.h"
#include "WindowsUtils/WriteProcessMemory.h"

namespace orbit_windows_utils {

namespace {

ErrorMessage GetLastErrorForFunction(std::string_view function) {
  return ErrorMessage(
      absl::StrFormat("Calling \"%s\": %s", function, orbit_base::GetLastErrorAsString()));
}

ErrorMessageOr<void> FlushInstructionCache(HANDLE process_handle, void* address, size_t size) {
  if (::FlushInstructionCache(process_handle, address, size) == 0) {
    return GetLastErrorForFunction("FlushInstructionCache");
  }
  return outcome::success();
}

ErrorMessageOr<void> WriteInstructionsAtAddress(const std::string& byte_buffer, void* address) {
  // Mark memory page for writing.
  DWORD old_protect = 0;
  if (!VirtualProtect(address, byte_buffer.size(), PAGE_EXECUTE_READWRITE, &old_protect)) {
    return GetLastErrorForFunction("VirtualProtect");
  }

  // Write original bytes.
  memcpy(address, byte_buffer.data(), byte_buffer.size());

  // Restore original page protection.
  if (!VirtualProtect(address, byte_buffer.size(), old_protect, &old_protect)) {
    return GetLastErrorForFunction("VirtualProtect");
  }

  OUTCOME_TRY(FlushInstructionCache(GetCurrentProcess(), address, byte_buffer.size()));
  return outcome::success();
}

}  // namespace

ErrorMessageOr<BusyLoopInfo> InstallBusyLoopAtAddress(HANDLE process_handle, void* address) {
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

ErrorMessageOr<void> SuspendThread(HANDLE thread_handle) {
  if (::SuspendThread(thread_handle) == (DWORD)-1) {
    return GetLastErrorForFunction("SuspendThread");
  }
  return outcome::success();
}

ErrorMessageOr<void> ResumeThread(HANDLE thread_handle) {
  if (::ResumeThread(thread_handle) == (DWORD)-1) {
    return GetLastErrorForFunction("ResumeThread");
  }
  return outcome::success();
}

ErrorMessageOr<void> RemoveBusyLoop(const BusyLoopInfo& busy_loop_info) {
  // Remove the busy loop and restore the original bytes.
  void* busy_loop_address = absl::bit_cast<void*>(busy_loop_info.address);
  OUTCOME_TRY(WriteInstructionsAtAddress(busy_loop_info.original_bytes, busy_loop_address));
  return outcome::success();
}

ErrorMessageOr<void> SetThreadInstructionPointer(HANDLE thread_handle,
                                                 uint64_t instruction_pointer) {
  CONTEXT c = {0};
  c.ContextFlags = CONTEXT_CONTROL;
  if (!GetThreadContext(thread_handle, &c)) {
    return GetLastErrorForFunction("GetThreadContext");
  }

  c.Rip = instruction_pointer;

  if (SetThreadContext(thread_handle, &c) == 0) {
    return GetLastErrorForFunction("SetThreadContext");
  }

  return outcome::success();
}

}  // namespace orbit_windows_utils
