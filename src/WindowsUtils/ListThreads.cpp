// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ListThreads.h"

#include <absl/base/casts.h>
#include <windows.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitBase/UniqueResource.h"
#include "WindowsUtils/SafeHandle.h"

// clang-format off
#include <psapi.h>
#include <tlhelp32.h>
// clang-format on

namespace orbit_windows_utils {

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
std::vector<Thread> ListThreads(uint32_t pid) {
  HANDLE thread_snap_handle = INVALID_HANDLE_VALUE;
  THREADENTRY32 thread_entry = {0};

  // Take a snapshot of all running threads.
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  thread_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (thread_snap_handle == INVALID_HANDLE_VALUE) {
    ORBIT_ERROR("Calling CreateToolhelp32Snapshot for threads");
    return {};
  }
  SafeHandle handle_closer(thread_snap_handle);

  // Retrieve information about the first thread.
  thread_entry.dwSize = sizeof(THREADENTRY32);
  if (!Thread32First(thread_snap_handle, &thread_entry)) {
    ORBIT_ERROR("Calling Thread32First for pid %u", pid);
    return {};
  }

  // Walk the thread list of the system and return the ones associated with "pid".
  std::vector<Thread> threads;
  do {
    // If pid is orbit_base::kInvalidProcessId, list all the threads on the system, otherwise,
    // use a valid pid to filter the threads list.
    if (pid == orbit_base::kInvalidProcessId || thread_entry.th32OwnerProcessID == pid) {
      Thread& thread = threads.emplace_back();
      thread.pid = pid;
      thread.tid = thread_entry.th32ThreadID;
      thread.name = orbit_base::GetThreadName(thread.tid);
    }
  } while (Thread32Next(thread_snap_handle, &thread_entry));

  return threads;
}

std::vector<Thread> ListAllThreads() { return ListThreads(orbit_base::kInvalidProcessId); }

}  // namespace orbit_windows_utils
