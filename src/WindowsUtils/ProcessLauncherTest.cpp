// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/ProcessLauncher.h"

namespace orbit_windows_utils {

namespace {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

[[nodiscard]] std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}

}  // namespace

TEST(ProcessLauncher, LaunchProcess) {
  ProcessLauncher launcher;
  auto launch_result =
      launcher.LaunchProcess(GetTestExecutablePath(), /*working_directory=*/"", /*arguments=*/"",
                             /*pause_at_entry_point*/ false);
  ASSERT_THAT(launch_result, HasNoError());
}

TEST(ProcessLauncher, LaunchSuspendResumeProcess) {
// TODO(https://github.com/google/orbit/issues/4503): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  ProcessLauncher launcher;
  auto launch_result =
      launcher.LaunchProcess(GetTestExecutablePath(), /*working_directory=*/"", /*arguments=*/"",
                             /*pause_at_entry_point*/ true);
  ASSERT_THAT(launch_result, HasNoError());
  uint32_t process_id = launch_result.value();

  auto suspend_result = launcher.SuspendProcessSpinningAtEntryPoint(process_id);
  ASSERT_THAT(suspend_result, HasNoError());

  auto resume_result = launcher.ResumeProcessSuspendedAtEntryPoint(process_id);
  ASSERT_THAT(resume_result, HasNoError());
}

TEST(ProcessLauncher, LaunchNonExistingProcess) {
  constexpr const char* non_existing_executable = R"(C:\non_existing_executable.exe)";
  ProcessLauncher launcher;
  auto result =
      launcher.LaunchProcess(non_existing_executable, /*working_directory=*/"", /*arguments=*/"",
                             /*pause_at_entry_point*/ false);
  ASSERT_THAT(result, HasError("Executable does not exist"));
}

TEST(ProcessLauncher, SuspendNonExistingProcess) {
  ProcessLauncher launcher;
  auto result = launcher.SuspendProcessSpinningAtEntryPoint(orbit_base::kInvalidProcessId);
  ASSERT_THAT(result, HasError("Trying to suspend unknown process"));
}

TEST(ProcessLauncher, ResumeNonExistingProcess) {
  ProcessLauncher launcher;
  auto result = launcher.ResumeProcessSuspendedAtEntryPoint(orbit_base::kInvalidProcessId);
  ASSERT_THAT(result, HasError("Trying to resume unknown process"));
}

}  // namespace orbit_windows_utils
