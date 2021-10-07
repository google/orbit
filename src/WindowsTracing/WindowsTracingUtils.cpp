// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsTracing/WindowsTracingUtils.h"

#include <ObjectUtils/CoffFile.h>
#include <OrbitBase/Logging.h>
#include <OrbitBase/Profiling.h>
#include <OrbitBase/ThreadUtils.h>
#include <OrbitBase/UniqueResource.h>
#include <absl/base/casts.h>
#include <windows.h>

// Include after windows.h.
#include <psapi.h>
#include <tlhelp32.h>

#include <filesystem>
#include <optional>

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ThreadName;
using orbit_object_utils::CoffFile;

namespace orbit_windows_tracing {

// Returns true if the process identified by "pid" is a 64 bit process.
// Note that this assumes we are a 64 bit process running on a 64 bit OS.
[[nodiscard]] static std::optional<bool> Is64Bit(HANDLE process_handle) {
  BOOL is_32_bit_process_on_64_bit_os = 0;
  if (IsWow64Process(process_handle, &is_32_bit_process_on_64_bit_os) != 0) {
    return is_32_bit_process_on_64_bit_os != TRUE;
  } else {
    ERROR("Calling IsWow64Process for pid %u.", GetProcessId(process_handle));
  }
  return std::nullopt;
}

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
std::vector<ProcessInfo> ListProcesses() {
  HANDLE process_snap_handle = INVALID_HANDLE_VALUE;
  HANDLE process_handle = INVALID_HANDLE_VALUE;
  PROCESSENTRY32 process_entry = {0};

  // Take a snapshot of all processes in the system.
  process_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (process_snap_handle == INVALID_HANDLE_VALUE) {
    ERROR("Calling CreateToolhelp32Snapshot.");
    return {};
  }
  orbit_base::unique_resource handle{process_snap_handle, [](HANDLE h) { CloseHandle(h); }};

  // Retrieve information about the first process, and exit if unsuccessful.
  process_entry.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(process_snap_handle, &process_entry)) {
    ERROR("Calling Process32First.");
    return {};
  }

  // Walk the snapshot of processes.
  std::vector<ProcessInfo> process_infos;
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

    ProcessInfo& process_info = process_infos.emplace_back();
    process_info.set_pid(pid);
    process_info.set_full_path(full_path);
    process_info.set_name(std::string(process_name_w.begin(), process_name_w.end()));
    process_info.set_is_64_bit(is_64_bit);

  } while (Process32Next(process_snap_handle, &process_entry));

  return process_infos;
}

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
std::vector<orbit_grpc_protos::ModuleInfo> ListModules(uint32_t pid) {
  HANDLE module_snap_handle = INVALID_HANDLE_VALUE;
  MODULEENTRY32 module_entry = {0};

  // Take a snapshot of all modules in the specified process.
  module_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  if (module_snap_handle == INVALID_HANDLE_VALUE) {
    ERROR("Calling CreateToolhelp32Snapshot for modules");
    return {};
  }
  orbit_base::unique_resource handle{module_snap_handle, [](HANDLE h) { CloseHandle(h); }};

  // Retrieve information about the first module.
  module_entry.dwSize = sizeof(MODULEENTRY32);
  if (!Module32First(module_snap_handle, &module_entry)) {
    ERROR("Calling Module32First for pid %u", pid);
    return {};
  }

  // Walk the module list of the process.
  std::vector<ModuleInfo> module_infos;
  do {
    std::wstring module_name_w = module_entry.szModule;
    std::wstring module_path_w = module_entry.szExePath;

    std::string build_id;
    std::string module_path = std::string(module_path_w.begin(), module_path_w.end());
    auto coff_file_or_error = orbit_object_utils::CreateCoffFile(module_path);
    if (coff_file_or_error.has_value()) {
      build_id = coff_file_or_error.value()->GetBuildId();
    } else {
      ERROR("Could not create Coff file for module %s, build-id will be empty.", module_path);
    }

    ModuleInfo& module_info = module_infos.emplace_back();
    module_info.set_name(std::string(module_name_w.begin(), module_name_w.end()));
    module_info.set_file_path(module_path);
    module_info.set_file_size(module_entry.modBaseSize);
    module_info.set_address_start(absl::bit_cast<uint64_t>(module_entry.modBaseAddr));
    module_info.set_address_end(module_info.address_start() + module_entry.modBaseSize);
    if (coff_file_or_error.has_value()) {
      module_info.set_build_id(coff_file_or_error.value()->GetBuildId());
    }

  } while (Module32Next(module_snap_handle, &module_entry));

  return module_infos;
}

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
std::vector<ThreadName> ListThreads(uint32_t pid) {
  HANDLE thread_snap_handle = INVALID_HANDLE_VALUE;
  THREADENTRY32 thread_entry = {0};

  // Take a snapshot of all running threads.
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  thread_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (thread_snap_handle == INVALID_HANDLE_VALUE) {
    ERROR("Calling CreateToolhelp32Snapshot for threads");
    return {};
  }
  orbit_base::unique_resource handle{thread_snap_handle, [](HANDLE h) { CloseHandle(h); }};

  // Retrieve information about the first thread.
  thread_entry.dwSize = sizeof(THREADENTRY32);
  if (!Thread32First(thread_snap_handle, &thread_entry)) {
    ERROR("Calling Thread32First for pid %u.", pid);
    return {};
  }

  // Walk the thread list of the system and return the ones associated with "pid".
  std::vector<ThreadName> thread_names;
  do {
    // If pid is orbit_base::kInvalidProcessId, list all the threads on the system, otherwise,
    // use a valid pid to filter the threads list.
    if (pid == orbit_base::kInvalidProcessId || thread_entry.th32OwnerProcessID == pid) {
      uint32_t tid = thread_entry.th32ThreadID;
      ThreadName& thread_name = thread_names.emplace_back();
      thread_name.set_pid(pid);
      thread_name.set_tid(tid);
      thread_name.set_name(orbit_base::GetThreadName(tid));
      thread_name.set_timestamp_ns(timestamp_ns);
    }
  } while (Thread32Next(thread_snap_handle, &thread_entry));

  return thread_names;
}

std::vector<ThreadName> ListAllThreads() { return ListThreads(orbit_base::kInvalidProcessId); }

}  // namespace orbit_windows_tracing
