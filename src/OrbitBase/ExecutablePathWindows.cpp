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
  char exe_file_name[kMaxPath] = {0};
  if (!GetModuleFileNameA(/*hModule=*/nullptr, exe_file_name, kMaxPath)) {
    FATAL("GetModuleFileName failed with: %s", GetLastErrorAsString());
  }

  // Clean up "../" inside full path
  char exe_full_path[kMaxPath] = {0};
  if (!GetFullPathNameA(exe_file_name, kMaxPath, exe_full_path, nullptr)) {
    FATAL("GetFullPathNameA failed with: %s", GetLastErrorAsString());
  }

  return std::filesystem::path(exe_full_path);
}

ErrorMessageOr<std::filesystem::path> GetExecutablePath(uint32_t pid) {
  HANDLE handle = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, /*bInheritHandle=*/FALSE, pid);
  if (handle == nullptr) {
    return ErrorMessage(absl::StrFormat("Error calling OpenProcess for pid %u", pid));
  }

  constexpr uint32_t kMaxPath = 2048;
  char exe_file_name[kMaxPath] = {0};

  if (!GetModuleFileNameExA(handle, /*hModule=*/nullptr, exe_file_name, kMaxPath)) {
    return ErrorMessage(
        absl::StrFormat("GetModuleFileNameExA failed with: %s", GetLastErrorAsString()));
  }

  // Clean up "../" inside full path
  char exe_full_path[kMaxPath] = {0};
  if (!GetFullPathNameA(exe_file_name, kMaxPath, exe_full_path, nullptr)) {
    return ErrorMessage(
        absl::StrFormat("GetFullPathNameA failed with: %s", GetLastErrorAsString()));
  }

  return std::filesystem::path(exe_full_path);
}

}  // namespace orbit_base
