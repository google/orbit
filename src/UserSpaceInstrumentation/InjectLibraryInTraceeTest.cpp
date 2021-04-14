// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/wait.h>

#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "MachineCode.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

void OpenUseAndCloseLibrary(pid_t pid) {
  // Stop the child process using our tooling.
  ASSERT_TRUE(AttachAndStopProcess(pid).has_value());

  // Tracee does not have the dynamic lib loaded, obviously.
  const std::string kLibName = "libUserSpaceInstrumentationTestLib.so";
  auto maps_before = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  ASSERT_TRUE(maps_before.has_value());
  EXPECT_FALSE(absl::StrContains(maps_before.value(), kLibName));

  // Load dynamic lib into tracee.
  auto result_dlopen =
      DlopenInTracee(pid, orbit_base::GetExecutableDir() / ".." / "lib" / kLibName, RTLD_NOW);
  ASSERT_TRUE(result_dlopen.has_value());

  // Tracee now does have the dynamic lib loaded.
  auto maps_after_open = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  ASSERT_TRUE(maps_after_open.has_value());
  EXPECT_TRUE(absl::StrContains(maps_after_open.value(), kLibName));

  // Look up symbol for "TrivialFunction" in the dynamic lib.
  auto result_dlsym = DlsymInTracee(pid, result_dlopen.value(), "TrivialFunction");
  ASSERT_TRUE(result_dlsym.has_value());
  const uint64_t address_function = absl::bit_cast<uint64_t>(result_dlsym.value());

  // Write machine code to call "TrivialFunction" from the dynamic lib.
  constexpr uint64_t kScratchPadSize = 1024;
  auto result_address_code = AllocateInTracee(pid, 0, kScratchPadSize);
  ASSERT_TRUE(result_address_code.has_value());
  const uint64_t address_code = result_address_code.value();
  // Move function's address to rax, do the call, and hit a breakpoint:
  // movabs rax, address_function     48 b8 address_function
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xb8})
      .AppendImmediate64(address_function)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  auto result_or_error = ExecuteMachineCode(pid, address_code, code);
  ASSERT_FALSE(result_or_error.has_error());
  EXPECT_EQ(42, result_or_error.value());

  ASSERT_FALSE(FreeInTracee(pid, address_code, kScratchPadSize).has_error());

  // Close the library.
  auto result_dlclose = DlcloseInTracee(pid, result_dlopen.value());
  ASSERT_FALSE(result_dlclose.has_error()) << result_dlclose.error().message();

  // Now, again, the lib is absent from the tracee.
  auto maps_after_close = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  ASSERT_TRUE(maps_after_close.has_value());
  EXPECT_FALSE(absl::StrContains(maps_after_close.value(), kLibName));

  ASSERT_TRUE(DetachAndContinueProcess(pid).has_value());
}

}  // namespace

TEST(InjectLibraryInTraceeTest, OpenUseAndCloseLibraryInUserCode) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

  OpenUseAndCloseLibrary(pid);

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(InjectLibraryInTraceeTest, OpenUseAndCloseLibraryInSyscall) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    while (true) {
      // Child will be stuck in syscall sys_clock_nanosleep.
      std::this_thread::sleep_for(std::chrono::hours(1000000000));
    }
  }

  OpenUseAndCloseLibrary(pid);

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation