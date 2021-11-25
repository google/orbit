// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/Injection.h"

namespace orbit_windows_utils {

namespace {

[[nodiscard]] std::filesystem::path GetTestDllPath() {
  return orbit_test::GetTestdataDir() / "libtest.dll";
}

[[nodiscard]] std::filesystem::path GetNonExistantDllPath() { return "Z:/non_existant_dll.dll"; }

}  // namespace

TEST(Injection, InjectDlInCurrentProcess) {
  // Injection.
  uint32_t pid = orbit_base::GetCurrentProcessId();
  ErrorMessageOr<void> result = InjectDll(pid, GetTestDllPath());
  if (result.has_error()) ERROR("Running InjectDlInCurrentProcess: %s", result.error().message());
  EXPECT_FALSE(result.has_error());

  // Re-injection.
  result = InjectDll(pid, GetTestDllPath());
  EXPECT_TRUE(result.has_error());
  EXPECT_THAT(result.error().message(), testing::HasSubstr("is already loaded in process"));

  // GetRemoteProcAddress.
  ErrorMessageOr<uint64_t> remote_proc_result =
      GetRemoteProcAddress(pid, "libtest.dll", "PrintHelloWorld");
  EXPECT_TRUE(remote_proc_result.has_value());

  // CreateRemoteThread.
  ErrorMessageOr<void> remote_thread_result =
      CreateRemoteThread(pid, "libtest.dll", "PrintHelloWorld", {});
  EXPECT_FALSE(remote_thread_result.has_error());
}

TEST(Injection, InjectNonExistantDll) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  ErrorMessageOr<void> result = InjectDll(pid, GetNonExistantDllPath());
  EXPECT_TRUE(result.has_error());
  EXPECT_THAT(result.error().message(), testing::HasSubstr("Path does not exist"));
}

}  // namespace orbit_windows_utils
