// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

// Include after windows.h.
#include <libloaderapi.h>

#include "WindowsUtils/ListProcesses.h"

namespace orbit_windows_utils {

TEST(ListProcesses, ContainsCurrentProcess) {
  std::vector<Process> processes = ListProcesses();
  EXPECT_NE(processes.size(), 0);

  char this_exe_file_name[MAX_PATH];
  GetModuleFileNameA(NULL, this_exe_file_name, MAX_PATH);

  bool found_this_exe = false;
  for (const Process& process : processes) {
    if (process.full_path == this_exe_file_name) {
      found_this_exe = true;
      break;
    }
  }

  EXPECT_TRUE(found_this_exe);
}

}  // namespace orbit_windows_utils
