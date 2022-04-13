// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#include "OrbitBase/ThreadUtils.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/BusyLoopLauncher.h"
#include "WindowsUtils/BusyLoopUtils.h"
#include "WindowsUtils/DllInjection.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/ProcessList.h"

namespace orbit_windows_utils {

namespace {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

[[nodiscard]] std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}

[[nodiscard]] std::filesystem::path GetTestDllPath() {
  return orbit_test::GetTestdataDir() / "libtest.dll";
}

__declspec(noinline) void IncrementCounter(uint32_t* counter, std::atomic<bool>* exit_requested) {
  ++(*counter);

  // Wait for exit request.
  while (!(*exit_requested)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void MaybeTerminateProcess(uint32_t process_id) {
  ErrorMessageOr<SafeHandle> process_handle =
      OpenProcess(PROCESS_ALL_ACCESS, /*inherit_handle=*/false, process_id);

  if (process_handle.has_value()) {
    ::TerminateProcess(*process_handle.value(), /*exit_code*/ 0);
  }
}

}  // namespace

TEST(BusyLoop, BusyLoopLauncher) {
  // Skipped due to http://b/228592301
  GTEST_SKIP();
  // Start process that would normally exit instantly and install busy loop at entry point.
  BusyLoopLauncher busy_loop_launcher;
  ErrorMessageOr<BusyLoopInfo> result = busy_loop_launcher.StartWithBusyLoopAtEntryPoint(
      GetTestExecutablePath(), /*working_directory=*/"", /*arguments=*/"");
  ASSERT_TRUE(result.has_value());

  // Validate returned busy loop info.
  const BusyLoopInfo& busy_loop_info = result.value();
  ASSERT_TRUE(busy_loop_info.original_bytes.size() > 0);
  ASSERT_NE(busy_loop_info.process_id, 0);
  ASSERT_NE(busy_loop_info.address, 0);

  // At this point, the process should be busy looping at entry point. Make sure it's still alive.
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  ASSERT_TRUE(process_list->GetProcessByPid(busy_loop_info.process_id).has_value());

  // Inject dll while process is spinning at entry point.
  ErrorMessageOr<void> injection_result = InjectDll(busy_loop_info.process_id, GetTestDllPath());
  ASSERT_THAT(injection_result, HasNoError());

  // Suspend main thread and replace busy loop by original bytes.
  ASSERT_THAT(busy_loop_launcher.SuspendMainThreadAndRemoveBusyLoop(), HasNoError());

  // Call function in injected dll.
  ErrorMessageOr<void> remote_thread_result =
      CreateRemoteThread(busy_loop_info.process_id, "libtest.dll", "PrintHelloWorld", {});
  EXPECT_THAT(remote_thread_result, HasNoError());

  // Resume main thread.
  ASSERT_THAT(busy_loop_launcher.ResumeMainThread(), HasNoError());

  // Kill process if not dead already.
  MaybeTerminateProcess(busy_loop_info.process_id);

  // Wait for created process to exit.
  busy_loop_launcher.WaitForProcessToExit();
}

TEST(BusyLoop, BusyLoopAtFunction) {
  std::atomic<bool> exit_requested = false;
  uint32_t counter = 0;

  // Install busy loop at start of "IncrementCounter" function.
  auto busy_loop_info_or = InstallBusyLoopAtAddress(GetCurrentProcess(), &IncrementCounter);
  ASSERT_TRUE(busy_loop_info_or.has_value());
  const BusyLoopInfo& busy_loop_info = busy_loop_info_or.value();

  // Launch a thread that calls "IncrementCounter" which should spin at function entry.
  std::thread t(&IncrementCounter, &counter, &exit_requested);

  // Verify that our counter has not been incremented.
  constexpr size_t kNumChecks = 10;
  for (int i = 0; i < kNumChecks; ++i) {
    ASSERT_EQ(counter, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Suspend thread and replace busy loop by original code.
  HANDLE thread_handle = t.native_handle();
  ASSERT_TRUE(orbit_windows_utils::SuspendThread(thread_handle).has_value());
  ASSERT_TRUE(RemoveBusyLoop(busy_loop_info_or.value()).has_value());

  // Make sure the instruction pointer is back to the original address.
  ASSERT_TRUE(SetThreadInstructionPointer(thread_handle, busy_loop_info.address).has_value());

  // Verify that our counter has still not been incremented.
  for (int i = 0; i < kNumChecks; ++i) {
    ASSERT_EQ(counter, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Resume thread.
  ASSERT_TRUE(orbit_windows_utils::ResumeThread(thread_handle).has_value());

  // Wait for thread to exit.
  exit_requested = true;
  t.join();

  // Make sure our counter was incremented.
  ASSERT_EQ(counter, 1);
}

}  // namespace orbit_windows_utils