// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ListProcesses.h"

#include <absl/base/casts.h>
#include <windows.h>

#include <filesystem>
#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"

// Include after windows.h.
#include <psapi.h>
#include <tlhelp32.h>

namespace orbit_windows_utils {

// Returns true if the process identified by "pid" is a 64 bit process.
// Note that this assumes we are a 64 bit process running on a 64 bit OS.
[[nodiscard]] static std::optional<bool> Is64Bit(HANDLE process_handle) {
  BOOL is_32_bit_process_on_64_bit_os = 0;
  if (IsWow64Process(process_handle, &is_32_bit_process_on_64_bit_os) != 0) {
    return is_32_bit_process_on_64_bit_os != TRUE;
  }
  ERROR("Calling IsWow64Process for pid %u", GetProcessId(process_handle));
  return std::nullopt;
}

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
std::vector<Process> ListProcesses() {
  HANDLE process_snap_handle = INVALID_HANDLE_VALUE;
  HANDLE process_handle = INVALID_HANDLE_VALUE;
  PROCESSENTRY32 process_entry = {0};

  // Take a snapshot of all processes in the system.
  process_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (process_snap_handle == INVALID_HANDLE_VALUE) {
    ERROR("Calling CreateToolhelp32Snapshot");
    return {};
  }
  orbit_base::unique_resource handle{process_snap_handle, [](HANDLE h) { CloseHandle(h); }};

  // Retrieve information about the first process, and exit if unsuccessful.
  process_entry.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(process_snap_handle, &process_entry)) {
    ERROR("Calling Process32First");
    return {};
  }

  // Walk the snapshot of processes.
  std::vector<Process> processes;
  do {
    uint32_t pid = process_entry.th32ProcessID;
    std::wstring process_name_w = process_entry.szExeFile;
    char full_path[MAX_PATH] = {0};
    process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    // Assume 64 bit as the default.
    bool is_64_bit = true;

    // "System" processes cannot be opened.
    if (process_handle) {
      std::optional<bool> result = Is64Bit(process_handle);
      if (result.has_value()) {
        is_64_bit = result.value();
      }

      DWORD buffer_size = sizeof(full_path);
      if (QueryFullProcessImageNameA(process_handle, 0, full_path, &buffer_size) == 0) {
        ERROR("Calling GetModuleFileNameExA for pid %u", pid);
      }

      CloseHandle(process_handle);
    }

    Process& process = processes.emplace_back();
    process.pid = pid;
    process.full_path = full_path;
    process.name = std::string(process_name_w.begin(), process_name_w.end());
    process.is_64_bit = is_64_bit;

  } while (Process32Next(process_snap_handle, &process_entry));

  return processes;
}

}  // namespace orbit_windows_utils
