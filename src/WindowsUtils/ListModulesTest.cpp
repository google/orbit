// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

// clang-format off
#include <libloaderapi.h>
// clang-format on

#include <algorithm>
#include <chrono>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/StringConversion.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ListThreads.h"

namespace orbit_windows_utils {

static std::string GetCurrentModuleName() {
  HMODULE module_handle = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModuleName,
                    &module_handle);
  ORBIT_CHECK(module_handle);
  wchar_t module_name[MAX_PATH] = {0};
  GetModuleFileNameW(module_handle, module_name, MAX_PATH);
  return absl::StrReplaceAll(orbit_base::ToStdString(module_name), {{"\\", "/"}});
}

TEST(ListModules, ContainsCurrentModule) {
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

}  // namespace orbit_windows_utils
