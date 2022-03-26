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

namespace orbit_windows_utils {

ErrorMessageOr<BusyLoopInfo> InstallBusyLoopAtEntryPoint(HANDLE process_handle,
                                                         uint32_t entry_point_thread_id,
                                                         uint64_t entry_point_address) {
  constexpr const char kBusyLoopCode[] = {0xEB, 0xFE};
  BusyLoopInfo busy_loop;
  busy_loop.address = entry_point_address;
  busy_loop.thread_id = entry_point_thread_id;

  // Copy original bytes before installing busy loop.
  OUTCOME_TRY(
      busy_loop.original_bytes,
      ReadProcessMemory(GetProcessId(process_handle), entry_point_address, sizeof(kBusyLoopCode)));

  // Install busy loop.
  void* entry_point = absl::bit_cast<void*>(entry_point_address);
  OUTCOME_TRY(WriteProcessMemory(process_handle, entry_point, kBusyLoopCode));

  // Flush instruction cache.
  if (FlushInstructionCache(process_handle, entry_point, sizeof(kBusyLoopCode)) == 0) {
    return orbit_base::GetLastError("Calling \"FlushInstructionCache\"");
  }

  return busy_loop;
}

void ProcessLauncher::OnCreateProcessDebugEvent(const DEBUG_EVENT& event) {
  HANDLE thread_handle = event.u.CreateProcessInfo.hThread;
  process_handle_ = event.u.CreateProcessInfo.hProcess;
  start_address_ = absl::bit_cast<uint64_t>(event.u.CreateProcessInfo.lpStartAddress);

  InstallBusyLoopAtEntryPoint(process_handle_, GetThreadId(event.u.CreateProcessInfo.hThread),
                              start_address_);
}

void ProcessLauncher::OnBreakpointDebugEvent(const DEBUG_EVENT& event) {}

}  // namespace orbit_windows_utils
