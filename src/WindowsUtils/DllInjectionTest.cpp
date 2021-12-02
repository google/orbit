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
#include "WindowsUtils/DllInjection.h"

namespace orbit_windows_utils {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

namespace {

[[nodiscard]] std::filesystem::path GetTestDllPath() {
  return orbit_test::GetTestdataDir() / "libtest.dll";
}

[[nodiscard]] std::filesystem::path GetNonExistentDllPath() { return "Z:/non_existent_dll.dll"; }

}  // namespace

TEST(DllInjection, InjectDllInCurrentProcess) {
  // Injection.
  uint32_t pid = orbit_base::GetCurrentProcessId();
  ErrorMessageOr<void> result = InjectDll(pid, GetTestDllPath());
  EXPECT_THAT(result, HasNoError());

  // Re-injection.
  result = InjectDll(pid, GetTestDllPath());
  EXPECT_THAT(result, HasError("is already loaded in process"));

  // GetRemoteProcAddress.
  ErrorMessageOr<uint64_t> remote_proc_result =
      GetRemoteProcAddress(pid, "libtest.dll", "PrintHelloWorld");
  EXPECT_THAT(remote_proc_result, HasNoError());

  // CreateRemoteThread.
  ErrorMessageOr<void> remote_thread_result =
      CreateRemoteThread(pid, "libtest.dll", "PrintHelloWorld", {});
  EXPECT_THAT(remote_thread_result, HasNoError());
}

TEST(DllInjection, InjectNonExistentDll) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  ErrorMessageOr<void> result = InjectDll(pid, GetNonExistentDllPath());
  EXPECT_THAT(result, HasError("Path does not exist"));
}

}  // namespace orbit_windows_utils
