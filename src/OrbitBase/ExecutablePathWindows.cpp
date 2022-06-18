// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <direct.h>
#include <windows.h>

// clang-format off
#include <psapi.h>
// clang-format on

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "absl/strings/str_replace.h"

namespace orbit_base {

std::filesystem::path GetExecutablePath() {
  constexpr uint32_t kMaxPath = 2048;
  wchar_t exe_file_name[kMaxPath] = {0};
  if (!GetModuleFileNameW(/*hModule=*/nullptr, exe_file_name, kMaxPath)) {
    ORBIT_FATAL("%s", GetLastErrorAsString("GetModuleFileNameW"));
  }

  // Clean up "../" inside full path
  wchar_t exe_full_path[kMaxPath] = {0};
  if (!GetFullPathNameW(exe_file_name, kMaxPath, exe_full_path, nullptr)) {
    ORBIT_FATAL("%s", GetLastErrorAsString("GetFullPathNameW"));
  }

  return std::filesystem::path(exe_full_path);
}

ErrorMessageOr<std::filesystem::path> GetExecutablePath(uint32_t pid) {
  HANDLE handle = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, /*bInheritHandle=*/FALSE, pid);
  if (handle == nullptr) {
    return ErrorMessage(absl::StrFormat("Error calling OpenProcess for pid %u", pid));
  }

  constexpr uint32_t kMaxPath = 2048;
  wchar_t exe_file_name[kMaxPath] = {0};

  if (!GetModuleFileNameExW(handle, /*hModule=*/nullptr, exe_file_name, kMaxPath)) {
    return orbit_base::GetLastErrorAsErrorMessage("GetModuleFileNameExW");
  }

  // Clean up "../" inside full path
  wchar_t exe_full_path[kMaxPath] = {0};
  if (!GetFullPathNameW(exe_file_name, kMaxPath, exe_full_path, nullptr)) {
    return orbit_base::GetLastErrorAsErrorMessage("GetFullPathNameW");
  }

  return std::filesystem::path(exe_full_path);
}

}  // namespace orbit_base
