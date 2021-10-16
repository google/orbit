// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <direct.h>
#include <windows.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "absl/strings/str_format.h"

// clang-format off
#include <psapi.h>
// clang-format on

namespace orbit_base {

std::filesystem::path GetExecutablePath() {
  constexpr uint32_t kMaxPath = 1024;
  char exe_file_name[kMaxPath] = {0};

  if (!GetModuleFileNameA(/*hModule=*/nullptr, exe_file_name, kMaxPath)) {
    ERROR("GetModuleFileNameA failed with: %u", GetLastError());
    return {};
  }

  // Clean up "../" inside full path
  char exe_full_path[kMaxPath] = {0};
  if (!GetFullPathNameA(exe_file_name, kMaxPath, exe_full_path, nullptr)) {
    ERROR("GetFullPathNameA failed with: %u", GetLastError());
    return {};
  }

  return std::filesystem::path(exe_full_path);
}

[[nodiscard]] ErrorMessageOr<std::filesystem::path> GetExecutablePath(uint32_t pid) {
  HANDLE handle = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, /*bInheritHandle=*/FALSE, pid);
  if (handle == nullptr) {
    return ErrorMessage(absl::StrFormat("Error calling OpenProcess for pid %u", pid));
  }

  constexpr uint32_t kMaxPath = 1024;
  char exe_file_name[kMaxPath] = {0};

  if (!GetModuleFileNameExA(handle, /*hModule=*/nullptr, exe_file_name, kMaxPath)) {
    return ErrorMessage(absl::StrFormat("GetModuleFileNameExA failed with: %u", GetLastError()));
  }

  // Clean up "../" inside full path
  char exe_full_path[kMaxPath] = {0};
  if (!GetFullPathNameA(exe_file_name, kMaxPath, exe_full_path, nullptr)) {
    return ErrorMessage(absl::StrFormat("GetFullPathNameA failed with: %u", GetLastError()));
  }

  return std::filesystem::path(exe_full_path);
}

}  // namespace orbit_base