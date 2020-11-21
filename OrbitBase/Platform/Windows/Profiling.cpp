// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include <OrbitBase/Platform/Windows/Profiling.h>
// clang-format on

#include <processthreadsapi.h>

#include <OrbitBase/Logging.h>
#include <string>

std::string GetThreadName(pid_t tid) {
  const std::string kEmptyString;

  // Get thread handle from tid.
  HANDLE thread_handle = OpenThread(READ_CONTROL, FALSE, tid);
  if (thread_handle == nullptr) {
    ERROR("Retrieving threand name for tid %u", tid);
    return kEmptyString;
  }

  // Get thread name from handle.
  PWSTR thread_name_pwstr;
  if (SUCCEEDED(GetThreadDescription(thread_handle, &thread_name_pwstr))) {
    std::wstring thread_name_w(thread_name_pwstr);
    LocalFree(thread_name_pwstr);
    std::string thread_name(thread_name_w.begin(), thread_name_w.end());
    return thread_name;
  }

  return kEmptyString;
}

void SetThreadName(const std::string& name) {
  std::wstring wide_name(name);
  if (!SUCCEEDED(SetThreadDescription(GetCurrentThread(), wide_name.c_str()))) {
    ERROR("Setting thread name %s", name);
  }
}
