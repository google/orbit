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
#include "WindowsTracing/WindowsTracingUtils.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ThreadName;

namespace orbit_windows_tracing {

static std::string GetCurrentModuleName() {
  HMODULE module_handle = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModuleName,
                    &module_handle);
  CHECK(module_handle);
  char module_name[MAX_PATH] = {0};
  GetModuleFileNameA(module_handle, module_name, MAX_PATH);
  return module_name;
}

TEST(WindowsTracingUtils, ListProcesses) {
  std::vector<ProcessInfo> processes = orbit_windows_tracing::ListProcesses();
  EXPECT_NE(processes.size(), 0);

  char this_exe_file_name[MAX_PATH];
  GetModuleFileNameA(NULL, this_exe_file_name, MAX_PATH);

  bool found_this_exe = false;
  for (const ProcessInfo& process : processes) {
    if (process.full_path() == this_exe_file_name) {
      found_this_exe = true;
      break;
    }
  }

  EXPECT_TRUE(found_this_exe);
}

TEST(WindowsTracingUtils, ListModules) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  std::vector<ModuleInfo> modules = orbit_windows_tracing::ListModules(pid);
  EXPECT_NE(modules.size(), 0);

  std::string this_module_name = GetCurrentModuleName();
  bool found_this_module = false;
  for (const ModuleInfo& module_info : modules) {
    if (module_info.file_path() == this_module_name) {
      found_this_module = true;
      break;
    }
  }
  EXPECT_TRUE(found_this_module);
}

TEST(WindowsTracingUtils, ListThreads) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint32_t tid = orbit_base::GetCurrentThreadId();
  constexpr const char* kThreadName = "WindowsTracingListThreads";
  orbit_base::SetCurrentThreadName(kThreadName);

  std::vector<ThreadName> thread_names = orbit_windows_tracing::ListThreads(pid);
  EXPECT_NE(thread_names.size(), 0);

  std::string this_thread_name;
  for (ThreadName& thread_name : thread_names) {
    if (thread_name.tid() == tid) {
      this_thread_name = thread_name.name();
      break;
    }
  }

  EXPECT_FALSE(this_thread_name.empty());
  EXPECT_STREQ(this_thread_name.c_str(), kThreadName);
}

TEST(WindowsTracingUtils, ListAllThreads) {
  uint32_t tid = orbit_base::GetCurrentThreadId();
  constexpr const char* kThreadName = "WindowsTracingListAllThread";
  orbit_base::SetCurrentThreadName(kThreadName);

  std::vector<orbit_grpc_protos::ThreadName> thread_names = orbit_windows_tracing::ListAllThreads();
  EXPECT_NE(thread_names.size(), 0);

  std::string this_thread_name;
  for (ThreadName& thread_name : thread_names) {
    if (thread_name.tid() == tid) {
      this_thread_name = thread_name.name();
    }
  }

  EXPECT_FALSE(this_thread_name.empty());
  EXPECT_STREQ(this_thread_name.c_str(), kThreadName);
}

}  // namespace orbit_windows_tracing
