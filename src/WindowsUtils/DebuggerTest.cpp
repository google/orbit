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

namespace {

std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}

using orbit_windows_utils::DebugEventListener;
using orbit_windows_utils::Debugger;
using orbit_windows_utils::ProcessInfo;

}  // namespace

struct MockListener : public DebugEventListener {
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

TEST(Debugger, LaunchProcess) {
  constexpr size_t kNumListeners = 10;
  std::vector<MockListener> mock_listeners(kNumListeners);
  std::vector<DebugEventListener*> listeners;
  for (MockListener& mock_listener : mock_listeners) {
    listeners.push_back(&mock_listener);
  }

  Debugger debugger(listeners);

  for (MockListener& mock_listener : mock_listeners) {
    EXPECT_CALL(mock_listener, OnCreateProcessDebugEvent).Times(1);
    EXPECT_CALL(mock_listener, OnExitProcessDebugEvent).Times(1);
    EXPECT_CALL(mock_listener, OnBreakpointDebugEvent).Times(1);
    EXPECT_CALL(mock_listener, OnCreateThreadDebugEvent).Times(testing::AtLeast(1));
    EXPECT_CALL(mock_listener, OnExitThreadDebugEvent).Times(testing::AtLeast(1));
    EXPECT_CALL(mock_listener, OnLoadDllDebugEvent).Times(testing::AnyNumber());
    EXPECT_CALL(mock_listener, OnUnLoadDllDebugEvent).Times(testing::AnyNumber());
    EXPECT_CALL(mock_listener, OnOutputStringDebugEvent).Times(testing::AnyNumber());
    EXPECT_CALL(mock_listener, OnExceptionDebugEvent).Times(testing::AnyNumber());
    EXPECT_CALL(mock_listener, OnRipEvent).Times(testing::AnyNumber());
  }

  const std::string kArguments = "--sleep_for_ms=20";
  auto result = debugger.Start(GetTestExecutablePath(), /*working_directory=*/"", kArguments);
  ASSERT_TRUE(result.has_value());

  Debugger::StartInfo& start_info = result.value();
  std::string expected_command_line = GetTestExecutablePath().string();
  if (!kArguments.empty()) expected_command_line += absl::StrFormat(" %s", kArguments);
  EXPECT_STREQ(start_info.command_line.c_str(), expected_command_line.c_str());

  // Wait for debugger to automatically detach upon process termination.
  debugger.Wait();
}

TEST(Debugger, NoListener) {
  Debugger debugger({});
  const std::string kArguments = "--sleep_for_ms=20";
  auto result = debugger.Start(GetTestExecutablePath(), /*working_directory=*/"", kArguments);
  EXPECT_THAT(result, orbit_test_utils::HasError("Debugger has no debug event listener"));
}

TEST(Debugger, NonExistingExecutable) {
  MockListener listener;
  Debugger debugger({&listener});
  const std::string executable = R"(C:\non_existing_executable.exe)";
  auto result = debugger.Start(executable, "", "");
  EXPECT_THAT(result, orbit_test_utils::HasError("Executable does not exist"));
}

TEST(Debugger, NonExistingWorkingDirectory) {
  MockListener listener;
  Debugger debugger({&listener});
  const std::string non_existing_working_directory = R"(C:\non_existing_directory)";
  auto result = debugger.Start(GetTestExecutablePath(), non_existing_working_directory, "");
  EXPECT_THAT(result, orbit_test_utils::HasError("Working directory does not exist"));
}
