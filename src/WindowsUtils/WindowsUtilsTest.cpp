// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

// Include after windows.h.
#include <libloaderapi.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/ListProcesses.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ListThreads.h"

namespace orbit_windows_utils {

static std::string GetCurrentModuleName() {
  HMODULE module_handle = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModuleName,
                    &module_handle);
  CHECK(module_handle);
  char module_name[MAX_PATH] = {0};
  GetModuleFileNameA(module_handle, module_name, MAX_PATH);
  return module_name;
}

TEST(WindowsUtils, ListProcesses) {
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

TEST(WindowsUtils, ListModules) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  std::vector<Module> modules = ListModules(pid);
  EXPECT_NE(modules.size(), 0);

  std::string this_module_name = GetCurrentModuleName();
  bool found_this_module = false;
  for (const Module& module : modules) {
    if (module.full_path == this_module_name) {
      found_this_module = true;
      break;
    }
  }
  EXPECT_TRUE(found_this_module);
}

TEST(WindowsUtils, ListThreads) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint32_t tid = orbit_base::GetCurrentThreadId();
  constexpr const char* kThreadName = "WindowsUtilsListThreads";
  orbit_base::SetCurrentThreadName(kThreadName);

  std::vector<Thread> threads = ListThreads(pid);
  EXPECT_NE(threads.size(), 0);

  std::string this_thread_name;
  for (const Thread& thread : threads) {
    if (thread.tid == tid) {
      this_thread_name = thread.name;
      break;
    }
  }

  EXPECT_FALSE(this_thread_name.empty());
  EXPECT_STREQ(this_thread_name.c_str(), kThreadName);
}

TEST(WindowsUtils, ListAllThreads) {
  uint32_t tid = orbit_base::GetCurrentThreadId();
  constexpr const char* kThreadName = "WindowsUtilsListAllThread";
  orbit_base::SetCurrentThreadName(kThreadName);

  std::vector<Thread> threads = ListAllThreads();
  EXPECT_NE(threads.size(), 0);

  std::string this_thread_name;
  for (const Thread& thread : threads) {
    if (thread.tid == tid) {
      this_thread_name = thread.name;
      break;
    }
  }

  EXPECT_FALSE(this_thread_name.empty());
  EXPECT_STREQ(this_thread_name.c_str(), kThreadName);
}

}  // namespace orbit_windows_tracing
