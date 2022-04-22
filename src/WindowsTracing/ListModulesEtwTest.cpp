// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

// clang-format off
#include <libloaderapi.h>
// clang-format on

#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsTracing/ListModulesEtw.h"

namespace orbit_windows_tracing {

using orbit_windows_utils::Module;

static std::string GetCurrentModulePath() {
  HMODULE module_handle = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModulePath,
                    &module_handle);
  ORBIT_CHECK(module_handle);
  char module_name[MAX_PATH] = {0};
  GetModuleFileNameA(module_handle, module_name, MAX_PATH);
  return module_name;
}

TEST(ListModules, ContainsCurrentModule) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  std::vector<Module> modules = ListModulesEtw(pid);
  EXPECT_NE(modules.size(), 0);

  // The full path return by ETW contains the device name, not the drive letter.
  std::string this_module_name = std::filesystem::path(GetCurrentModulePath()).filename().string();
  bool found_this_module = false;
  for (const Module& module : modules) {
    if (module.name == this_module_name) {
      found_this_module = true;
      break;
    }
  }
  EXPECT_TRUE(found_this_module);
}

}  // namespace orbit_windows_tracing
