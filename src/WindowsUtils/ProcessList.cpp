// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ProcessList.h"

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <absl/synchronization/mutex.h>
#include <windows.h>

#include <filesystem>
#include <optional>

#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/StringConversion.h"
#include "OrbitBase/UniqueResource.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/SafeHandle.h"

// clang-format off
#include <processthreadsapi.h>
#include <psapi.h>
#include <tlhelp32.h>
// clang-format on

#include <algorithm>
#include <cstdint>

namespace orbit_windows_utils {

namespace {

// Returns true if the process identified by "process_handle" is a 64 bit process.
// Note that this assumes we are a 64 bit process running on a 64 bit OS.
[[nodiscard]] std::optional<bool> Is64Bit(HANDLE process_handle) {
  BOOL is_32_bit_process_on_64_bit_os = 0;
  if (IsWow64Process(process_handle, &is_32_bit_process_on_64_bit_os) != 0) {
    return is_32_bit_process_on_64_bit_os != TRUE;
  }
  ORBIT_ERROR("Calling IsWow64Process for pid %u: %s", GetProcessId(process_handle),
              orbit_base::GetLastErrorAsString());
  return std::nullopt;
}

[[nodiscard]] inline uint64_t FiletimeToUint64(FILETIME t) {
  return (static_cast<uint64_t>(t.dwHighDateTime) << 32) | static_cast<uint64_t>(t.dwLowDateTime);
}

// FILETIME contains a 64-bit value representing the number of 100-nanosecond intervals since
// January 1, 1601 (UTC).
//
// From https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime:
// "It is not recommended that you add and subtract values from the FILETIME structure to obtain
// relative times. Instead, you should copy the low- and high-order parts of the file time to a
// ULARGE_INTEGER structure, perform 64-bit arithmetic on the QuadPart member, and copy the
// LowPart and HighPart members into the FILETIME structure."
[[nodiscard]] inline uint64_t FileTimeDiffNs(FILETIME file_time_0, FILETIME file_time_1) {
  uint64_t t_0 = FiletimeToUint64(file_time_0);
  uint64_t t_1 = FiletimeToUint64(file_time_1);
  ORBIT_CHECK(t_1 >= t_0);
  constexpr uint64_t kIntervalNs = 100;
  return (t_1 - t_0) * kIntervalNs;
}

}  // namespace

class ProcessListImpl final : public ProcessList {
 public:
  ProcessListImpl() = default;
  ErrorMessageOr<void> Refresh() override;
  [[nodiscard]] std::vector<const Process*> GetProcesses() const override;
  [[nodiscard]] std::optional<const Process*> GetProcessByPid(uint32_t pid) const override;
  void UpdateCpuUsage();

 private:
  struct CpuUsage {
    uint64_t last_timestamp_ns = 0;
    FILETIME last_kernel_file_time = {};
    FILETIME last_user_file_time = {};
  };

  struct ProcessInfo {
    Process process;
    CpuUsage cpu_usage;
    bool is_process_alive = true;
    DWORD open_process_error = 0;
  };

  absl::flat_hash_map<uint32_t, ProcessInfo> process_infos_;
};

std::vector<const Process*> ProcessListImpl::GetProcesses() const {
  std::vector<const Process*> processes;

  processes.reserve(process_infos_.size());
  for (const auto& [unused_pid, process_info] : process_infos_) {
    processes.push_back(&process_info.process);
  }

  std::sort(processes.begin(), processes.end(), [](const Process* a, const Process* b) {
    return a->cpu_usage_percentage > b->cpu_usage_percentage;
  });

  return processes;
}

std::optional<const Process*> ProcessListImpl::GetProcessByPid(uint32_t pid) const {
  auto it = process_infos_.find(pid);
  if (it != process_infos_.end()) {
    return &it->second.process;
  }
  return std::nullopt;
}

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
ErrorMessageOr<void> ProcessListImpl::Refresh() {
  // Mark all existing processes as not alive.
  for (auto& [unused_pid, process_info] : process_infos_) {
    process_info.is_process_alive = false;
  }

  PROCESSENTRY32 process_entry = {0};

  // Take a snapshot of all processes in the system.
  HANDLE process_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, /*th32ProcessID=*/0);
  SafeHandle handle_closer(process_snap_handle);
  if (process_snap_handle == INVALID_HANDLE_VALUE) {
    return orbit_base::GetLastErrorAsErrorMessage("CreateToolhelp32Snapshot");
  }

  // Retrieve information about the first process, and exit if unsuccessful.
  process_entry.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(process_snap_handle, &process_entry)) {
    return orbit_base::GetLastErrorAsErrorMessage("Process32First");
  }

  // Walk the snapshot of processes.
  do {
    uint32_t pid = process_entry.th32ProcessID;
    auto it = process_infos_.find(pid);
    if (it == process_infos_.end()) {
      ProcessInfo& process_info = process_infos_[pid];
      std::string process_name = orbit_base::ToStdString(process_entry.szExeFile);
      wchar_t full_path[MAX_PATH] = {0};
      // Assume 64 bit as the default.
      bool is_64_bit = true;

      HANDLE handle = ::OpenProcess(PROCESS_ALL_ACCESS, /*bInheritHandle=*/FALSE, pid);
      SafeHandle handle_closer(handle);
      if (handle == nullptr) {
        // "System" processes cannot be opened, track errors to skip further OpenProcess calls.
        process_info.open_process_error = ::GetLastError();
        ORBIT_ERROR("Calling OpenProcess for %s[%u]: %s", process_name, pid,
                    orbit_base::GetLastErrorAsString());
      } else {
        std::optional<bool> result = Is64Bit(handle);
        if (result.has_value()) {
          is_64_bit = result.value();
        }

        DWORD num_chars = sizeof(full_path) / sizeof(full_path[0]);
        if (QueryFullProcessImageNameW(handle, 0, full_path, &num_chars) == 0) {
          ORBIT_ERROR("Calling GetModuleFileNameExA for pid %u", pid);
        }
      }

      Process& process = process_info.process;
      process.pid = pid;
      process.full_path = orbit_base::ToStdString(full_path);
      process.name = process_name;
      process.is_64_bit = is_64_bit;
    } else {
      // The process was already in the list, mark it as still alive.
      it->second.is_process_alive = true;
    }

  } while (Process32Next(process_snap_handle, &process_entry));

  // Erase stale processes.
  for (auto it = process_infos_.begin(), end = process_infos_.end(); it != end;) {
    // `erase()` will invalidate `it`, so advance `it` first.
    auto copy_it = it++;
    if (!copy_it->second.is_process_alive) {
      process_infos_.erase(copy_it);
    }
  }

  UpdateCpuUsage();

  return outcome::success();
}

void ProcessListImpl::UpdateCpuUsage() {
  for (auto& [pid, process_info] : process_infos_) {
    Process& process = process_info.process;

    // Don't call OpenProcess if it failed previously.
    if (process_info.open_process_error != 0) {
      continue;
    }

    ErrorMessageOr<SafeHandle> safe_handle_or_error =
        OpenProcess(PROCESS_ALL_ACCESS, /*bInheritHandle=*/FALSE, pid);

    if (safe_handle_or_error.has_error()) {
      process_info.open_process_error = ::GetLastError();
      ORBIT_ERROR("Calling OpenProcess for %s[%u]: %s", process.name, pid,
                  orbit_base::GetLastErrorAsString());
      continue;
    }

    HANDLE process_handle = *safe_handle_or_error.value();
    FILETIME creation_file_time = {};
    FILETIME exit_file_time = {};
    FILETIME kernel_file_time = {};
    FILETIME user_file_time = {};
    if (GetProcessTimes(process_handle, &creation_file_time, &exit_file_time, &kernel_file_time,
                        &user_file_time) == FALSE) {
      ORBIT_ERROR("Calling GetProcessTimes for %s[%u]: %s", process.name, process.pid,
                  orbit_base::GetLastErrorAsString());
      continue;
    }

    uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
    CpuUsage& cpu_usage = process_info.cpu_usage;
    bool is_first_update = cpu_usage.last_timestamp_ns == 0;

    uint64_t elapsed_ns = timestamp_ns - cpu_usage.last_timestamp_ns;
    uint64_t elapsed_kernel_ns = FileTimeDiffNs(cpu_usage.last_kernel_file_time, kernel_file_time);
    uint64_t elapsed_user_ns = FileTimeDiffNs(cpu_usage.last_user_file_time, user_file_time);
    cpu_usage.last_timestamp_ns = timestamp_ns;
    cpu_usage.last_kernel_file_time = kernel_file_time;
    cpu_usage.last_user_file_time = user_file_time;

    // We need at least two updates for meaningful data.
    if (is_first_update) continue;

    double cpu_time_ns = static_cast<double>(elapsed_kernel_ns + elapsed_user_ns);
    process.cpu_usage_percentage = 100.0 * (cpu_time_ns / static_cast<double>(elapsed_ns));
  }
}

std::unique_ptr<ProcessList> ProcessList::Create() {
  auto process_list = std::make_unique<ProcessListImpl>();
  ErrorMessageOr<void> result = process_list->Refresh();
  if (result.has_error()) {
    ORBIT_ERROR("Refreshing process list: %s", result.error().message());
  }
  return std::move(process_list);
}

}  // namespace orbit_windows_utils
