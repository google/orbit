// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <csignal>
#include <string>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "InjectLibraryInTracee.h"
#include "MachineCode.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/RegisterState.h"

namespace orbit_user_space_instrumentation {

TEST(InjectLibraryInTraceeTest, FindFunctionAddress) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

  // Stop the child process using our tooling.
  ASSERT_TRUE(AttachAndStopProcess(pid).has_value());

  auto function_address_or_error = FindFunctionAddress(pid, "printf", "libc-");
  EXPECT_TRUE(function_address_or_error.has_value());

  function_address_or_error = FindFunctionAddress(pid, "NOT_A_SYMBOL", "libc-");
  EXPECT_TRUE(function_address_or_error.has_error());
  EXPECT_TRUE(absl::StrContains(function_address_or_error.error().message(),
                                "Unable to locate function symbol"));

  function_address_or_error = FindFunctionAddress(pid, "printf", "NOT_A_LIB-");
  EXPECT_TRUE(function_address_or_error.has_error());
  EXPECT_TRUE(absl::StrContains(function_address_or_error.error().message(),
                                "There is no module NOT_A_LIB- in process"));

  function_address_or_error = FindFunctionAddress(-1, "printf", "libc-");
  EXPECT_TRUE(function_address_or_error.has_error());
  EXPECT_TRUE(
      absl::StrContains(function_address_or_error.error().message(), "Unable to open file"));

  // Detach and end child.
  ASSERT_TRUE(DetachAndContinueProcess(pid).has_value());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(InjectLibraryInTraceeTest, OpenUseCloseLibrary) {
  pid_t pid = fork();
  ASSERT_TRUE(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

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
  EXPECT_TRUE(result_dlsym.value() != nullptr);

  // Write machine code to call "TrivialFunction" from the dynamic lib.
  auto result_address_function = FindFunctionAddress(pid, "TrivialFunction", kLibName);
  ASSERT_TRUE(result_address_function.has_value());
  const uint64_t address_function = result_address_function.value();
  RegisterState original_registers;
  ASSERT_FALSE(original_registers.BackupRegisters(pid).has_error());
  constexpr uint64_t kScratchPadSize = 1024;
  auto result_address_code = AllocateInTracee(pid, kScratchPadSize);
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
  auto result_write_code = WriteTraceesMemory(pid, address_code, code.GetResultAsVector());
  ASSERT_FALSE(result_write_code.has_error());

  // Set rip to the code address and execute.
  RegisterState registers_set_rip = original_registers;
  registers_set_rip.GetGeneralPurposeRegisters()->x86_64.rip = address_code;
  ASSERT_FALSE(registers_set_rip.RestoreRegisters().has_error());
  ASSERT_EQ(0, ptrace(PTRACE_CONT, pid, 0, 0));
  ASSERT_EQ(pid, waitpid(pid, nullptr, 0));

  // Get return value and check result.
  RegisterState return_value_registers;
  ASSERT_FALSE(return_value_registers.BackupRegisters(pid).has_error());
  EXPECT_EQ(42, return_value_registers.GetGeneralPurposeRegisters()->x86_64.rax);

  ASSERT_FALSE(original_registers.RestoreRegisters().has_error());
  ASSERT_FALSE(FreeInTracee(pid, address_code, kScratchPadSize).has_error());

  // Close the library.
  auto result_dlclose = DlcloseInTracee(pid, result_dlopen.value());
  ASSERT_FALSE(result_dlclose.has_error());

  // Now, again, the lib is absent from the tracee.
  auto maps_after_close = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  ASSERT_TRUE(maps_after_close.has_value());
  EXPECT_FALSE(absl::StrContains(maps_after_close.value(), kLibName));

  // Detach and end child.
  ASSERT_TRUE(DetachAndContinueProcess(pid).has_value());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation