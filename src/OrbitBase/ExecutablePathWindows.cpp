// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <direct.h>
#include <windows.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "absl/strings/str_replace.h"

namespace orbit_base {

std::filesystem::path GetExecutablePath() {
  char exe_file_name[2048] = {0};
  LPSTR pszBuffer = exe_file_name;
  DWORD dwMaxChars = _countof(exe_file_name);
  DWORD dwLength = 0;

  if (!GetModuleFileNameA(NULL, pszBuffer, dwMaxChars)) {
    FATAL("GetModuleFileName failed with: %d", GetLastError());
  }

  // Clean up "../" inside full path
  char exe_full_path[MAX_PATH];
  if (!GetFullPathNameA(exe_file_name, MAX_PATH, exe_full_path, nullptr)) {
    FATAL("GetFullPathNameA failed with: %d", GetLastError());
  }

  return std::filesystem::path(exe_full_path);
}

}  // namespace orbit_base