// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#include "OrbitBase/StringConversion.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/Debugger.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/ProcessList.h"

namespace {

struct MockListener : public orbit_windows_utils::DebugEventListener {
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

std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}

using orbit_windows_utils::DebugEventListener;
using orbit_windows_utils::Debugger;
using orbit_windows_utils::ProcessInfo;
using orbit_windows_utils::ProcessList;
using orbit_windows_utils::SafeHandle;

}  // namespace

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

  const Debugger::StartInfo& start_info = result.value();
  std::string expected_command_line = GetTestExecutablePath().string();
  if (!kArguments.empty()) expected_command_line += absl::StrFormat(" %s", kArguments);
  EXPECT_EQ(start_info.command_line, expected_command_line);

  // Wait for debugger to automatically detach upon process termination.
  debugger.Wait();

  // Make sure process is not still runnning.
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  EXPECT_FALSE(process_list->GetProcessByPid(start_info.process_id).has_value());
}

TEST(Debugger, Detach) {
  MockListener mock_listener;
  Debugger debugger({&mock_listener});

  EXPECT_CALL(mock_listener, OnCreateProcessDebugEvent).Times(1);
  EXPECT_CALL(mock_listener, OnExitProcessDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(mock_listener, OnBreakpointDebugEvent).Times(1);
  EXPECT_CALL(mock_listener, OnCreateThreadDebugEvent).Times(testing::AtLeast(1));
  EXPECT_CALL(mock_listener, OnExitThreadDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(mock_listener, OnLoadDllDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(mock_listener, OnUnLoadDllDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(mock_listener, OnOutputStringDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(mock_listener, OnExceptionDebugEvent).Times(testing::AnyNumber());
  EXPECT_CALL(mock_listener, OnRipEvent).Times(testing::AnyNumber());

  // Start process which sleeps indefinitely as debuggee.
  constexpr const char* kArguments = "--infinite_sleep=true";
  auto result = debugger.Start(GetTestExecutablePath(), /*working_directory=*/"", kArguments);
  ASSERT_TRUE(result.has_value());
  const Debugger::StartInfo& start_info = result.value();

  // Detach debugger from debuggee.
  debugger.Detach();

  // Make sure the debugger thread exits.
  debugger.Wait();

  // Make sure debuggee is still runnning after detach operation.
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  EXPECT_TRUE(process_list->GetProcessByPid(start_info.process_id).has_value());

  // Terminate debuggee.
  auto safe_handle_or_error = orbit_windows_utils::OpenProcess(
      PROCESS_ALL_ACCESS, /*inherit_handle=*/false, start_info.process_id);
  ASSERT_TRUE(safe_handle_or_error.has_value());
  const SafeHandle& process_handle = safe_handle_or_error.value();
  EXPECT_NE(TerminateProcess(*process_handle, /*exit_code*/ 0), 0)
      << orbit_base::GetLastErrorAsString("TerminateProcess");
}

TEST(Debugger, NoListener) { EXPECT_DEATH(Debugger({}), "Check failed"); }

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
