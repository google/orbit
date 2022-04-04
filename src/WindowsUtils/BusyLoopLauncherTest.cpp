// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#include "TestUtils/TestUtils.h"
#include "WindowsUtils/BusyLoopLauncher.h"
#include "WindowsUtils/BusyLoopUtils.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/ProcessList.h"

namespace {

using orbit_windows_utils::BusyLoopInfo;
using orbit_windows_utils::BusyLoopLauncher;
using orbit_windows_utils::OpenProcess;
using orbit_windows_utils::ProcessList;
using orbit_windows_utils::SafeHandle;

[[nodiscard]] std::filesystem::path GetTestExecutablePath() {
  static auto path = orbit_base::GetExecutableDir() / "FakeCliProgram.exe";
  return path;
}

__declspec(noinline) void IncrementCounter(uint32_t* counter, std::atomic<bool>* exit_requested) {
  ++(*counter);

  // Wait for exit request.
  while (!(*exit_requested)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

}  // namespace

TEST(BusyLoop, BusyLoopLauncher) {
  // Start process that would normally exit instantly and install busy loop at entry point.
  BusyLoopLauncher busy_loop_launcher;
  const std::string kArguments = "";
  ErrorMessageOr<BusyLoopInfo> result = busy_loop_launcher.StartWithBusyLoopAtEntryPoint(
      GetTestExecutablePath(), /*working_directory=*/"", kArguments);
  ASSERT_TRUE(result.has_value());

  // Validate returned busy loop info.
  const BusyLoopInfo& busy_loop_info = result.value();
  EXPECT_TRUE(busy_loop_info.original_bytes.size() > 0);
  EXPECT_NE(busy_loop_info.process_id, 0);
  EXPECT_NE(busy_loop_info.address, 0);

  // At this point, the process should be busy looping at entry point.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Make sure the process is still alive.
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  EXPECT_TRUE(process_list->GetProcessByPid(busy_loop_info.process_id).has_value());

  // Kill process.
  auto open_result =
      OpenProcess(PROCESS_ALL_ACCESS, /*inherit_handle=*/false, busy_loop_info.process_id);
  ASSERT_TRUE(open_result.has_value());
  const SafeHandle& process_handle = open_result.value();
  EXPECT_NE(TerminateProcess(*process_handle, /*exit_code*/ 0), 0)
      << orbit_base::GetLastErrorAsString();
}

TEST(BusyLoop, BusyLoopAtFunction) {
  std::atomic<bool> exit_requested = false;
  uint32_t counter = 0;

  // Install busy loop at start of "IncrementCounter" function.
  auto busy_loop_info_or =
      orbit_windows_utils::InstallBusyLoopAtAddress(GetCurrentProcess(), &IncrementCounter);
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
  ASSERT_TRUE(orbit_windows_utils::RemoveBusyLoop(busy_loop_info_or.value()).has_value());

  // Make sure the instruction pointer is back to the original address.
  ASSERT_TRUE(
      orbit_windows_utils::SetThreadInstructionPointer(thread_handle, busy_loop_info.address)
          .has_value());

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
