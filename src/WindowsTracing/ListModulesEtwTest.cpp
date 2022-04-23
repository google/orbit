// Copyright (c) 2022 The Orbit Authors. All rights reserved.
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
#include "OrbitBase/ExecutablePath.h"
#include "WindowsTracing/ListModulesEtw.h"
#include "WindowsUtils/PathConverter.h"

namespace orbit_windows_tracing {

using orbit_windows_utils::Module;

TEST(ListModulesEtw, ContainsCurrentExecutable) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  std::vector<Module> modules = ListModulesEtw(pid);
  EXPECT_NE(modules.size(), 0);

  std::filesystem::path this_module_path = orbit_base::GetExecutablePath();
  bool found_this_module = false;
  for (const Module& module : modules) {
    ORBIT_LOG("Module name: %s", module.name);
    ORBIT_LOG("Module full_path: %s", module.full_path);
    if (module.full_path == this_module_path) {
      found_this_module = true;
      break;
    }
  }
  EXPECT_TRUE(found_this_module);
}

}  // namespace orbit_windows_tracing
