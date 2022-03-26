// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/StringConversion.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/Debugger.h"

using orbit_windows_utils::Debugger;
using orbit_windows_utils::ProcessInfo;

class MockDebugger : public Debugger {
 public:
  MOCK_METHOD(void, OnCreateProcessDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnExitProcessDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnCreateThreadDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnExitThreadDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnLoadDllDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnUnLoadDllDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnBreakpointDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnOutputStringDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnExceptionDebugEvent, (const DEBUG_EVENT& event), (override));
  MOCK_METHOD(void, OnRipEvent, (const DEBUG_EVENT& event), (override));
};

namespace {
std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}
}  // namespace

TEST(Debugger, LaunchProcess) {
  MockDebugger debugger;

  EXPECT_CALL(debugger, OnCreateProcessDebugEvent).Times(1);
  EXPECT_CALL(debugger, OnExitProcessDebugEvent).Times(1);
  EXPECT_CALL(debugger, OnBreakpointDebugEvent).Times(1);
  EXPECT_CALL(debugger, OnCreateThreadDebugEvent).Times(testing::AtLeast(1));
  EXPECT_CALL(debugger, OnExitThreadDebugEvent).Times(testing::AtLeast(1));
  EXPECT_CALL(debugger, OnLoadDllDebugEvent).Times(testing::AtLeast(1));
  EXPECT_CALL(debugger, OnUnLoadDllDebugEvent).Times(testing::AtLeast(1));
  EXPECT_CALL(debugger, OnOutputStringDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(debugger, OnExceptionDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(debugger, OnRipEvent).Times(testing::AnyNumber());

  auto result = debugger.Start(GetTestExecutablePath(), /*working_directory=*/"", /*arguments=*/"");
  ASSERT_TRUE(result.has_value());
  ProcessInfo& process_info = result.value();
  EXPECT_STREQ(process_info.command_line.c_str(), GetTestExecutablePath().string().c_str());

  // Wait for process to finish.
  WaitForSingleObject(*process_info.process_handle, INFINITE);

  debugger.Detach();
}

TEST(Debugger, NonExistingExecutable) {
  MockDebugger debugger;
  const std::string executable = R"(C:\non_existing_executable.exe)";
  auto result = debugger.Start(executable, "", "");
  EXPECT_THAT(result, orbit_test_utils::HasError("Executable does not exist"));
}

TEST(Debugger, NonExistingWorkingDirectory) {
  MockDebugger debugger;
  const std::string non_existing_working_directory = R"(C:\non_existing_directory)";
  auto result = debugger.Start(GetTestExecutablePath(), non_existing_working_directory, "");
  EXPECT_THAT(result, orbit_test_utils::HasError("Working directory does not exist"));
}
