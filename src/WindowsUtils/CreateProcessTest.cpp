// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/CreateProcess.h"
#include "WindowsUtils/ProcessList.h"

namespace {
std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}

using orbit_windows_utils::ProcessInfo;
using orbit_windows_utils::ProcessList;

}  // namespace

TEST(CreateProcess, SuccessfulProcessCreation) {
  constexpr const char* kArguments = "--infinite_sleep=true";
  auto result = orbit_windows_utils::CreateProcess(GetTestExecutablePath(), "", kArguments);
  ASSERT_FALSE(result.has_error());
  ProcessInfo& process_info = result.value();
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  EXPECT_TRUE(process_list->GetProcessByPid(process_info.process_id).has_value());
  EXPECT_NE(TerminateProcess(*process_info.process_handle, /*exit_code*/ 0), 0)
      << orbit_base::GetLastErrorAsString("TerminateProcess");
}

TEST(CreateProcess, CommandLine) {
  constexpr const char* kArguments = "--sleep_for_ms=20";
  auto result = orbit_windows_utils::CreateProcess(GetTestExecutablePath(), "", kArguments);
  ASSERT_FALSE(result.has_error());
  ProcessInfo& process_info = result.value();
  std::string expected_command_line =
      absl::StrFormat("%s %s", GetTestExecutablePath().string(), kArguments);
  EXPECT_EQ(process_info.command_line, expected_command_line);
}

TEST(CreateProcess, WorkingDirectory) {
  auto result =
      orbit_windows_utils::CreateProcess(GetTestExecutablePath(), orbit_test::GetTestdataDir(), "");
  ASSERT_FALSE(result.has_error());
  ProcessInfo& process_info = result.value();
  EXPECT_EQ(process_info.working_directory, orbit_test::GetTestdataDir().string());
}

TEST(CreateProcess, NonExistingExecutable) {
  const std::string executable = R"(C:\non_existing_executable.exe)";
  auto result = orbit_windows_utils::CreateProcess(executable, "", "");
  EXPECT_THAT(result, orbit_test_utils::HasError("Executable does not exist"));
}

TEST(CreateProcess, NonExistingWorkingDirectory) {
  const std::string non_existing_working_directory = R"(C:\non_existing_directory)";
  auto result = orbit_windows_utils::CreateProcess(GetTestExecutablePath(),
                                                   non_existing_working_directory, "");
  EXPECT_THAT(result, orbit_test_utils::HasError("Working directory does not exist"));
}