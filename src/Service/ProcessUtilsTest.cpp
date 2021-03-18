// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>

#include <algorithm>

#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "ProcessUtils.h"
#include "module.pb.h"

TEST(ProcessUtils, CreateModuleFromProcessMemory) {
  const auto maps_data = orbit_elf_utils::ReadProcMapsFile(getpid());
  ASSERT_TRUE(maps_data.has_value());

  const auto maps = orbit_elf_utils::ParseMaps(maps_data.value());

  const auto is_vdso = [](const orbit_elf_utils::MapEntry& entry) {
    return entry.module_path == "[vdso]";
  };

  const auto it = std::find_if(maps.begin(), maps.end(), is_vdso);

  if (it == maps.end()) {
    GTEST_SKIP() << "The test process has no [vdso] module, so we can't test loading it.";
  }

  const auto module = orbit_service::CreateModuleFromProcessMemory(getpid(), *it);
  if (module.has_error() &&
      absl::StrContains(module.error().message(), "Operation not permitted")) {
    GTEST_SKIP() << absl::StrFormat("Can't perform the test due to missing PTRACE privileges: %s",
                                    module.error().message());
  }
  ASSERT_FALSE(module.has_error()) << module.error().message();

  EXPECT_EQ(module.value().name(), "[vdso]");
  EXPECT_EQ(module.value().load_bias(), 0x0);
}

TEST(ProcessUtils, ReadModulesFromProcMaps) {
  const auto modules_or_error = orbit_service::ReadModulesFromProcMaps(getpid());
  ASSERT_FALSE(modules_or_error.has_error()) << modules_or_error.error().message();

  const auto& modules = modules_or_error.value();
  EXPECT_GE(modules.size(), 2);  // At least the test-executable, and libc.

  bool has_test_executable = false;
  bool has_libc = false;

  for (const orbit_grpc_protos::ModuleInfo& module : modules) {
    if (module.name() == orbit_base::GetExecutablePath().filename().string()) {
      has_test_executable = true;
      continue;
    }
    if (absl::StartsWith(module.name(), "libc")) {
      has_libc = true;
      continue;
    }
  }

  EXPECT_TRUE(has_test_executable);
  EXPECT_TRUE(has_libc);
}