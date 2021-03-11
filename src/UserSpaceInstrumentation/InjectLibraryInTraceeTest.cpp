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

TEST(InjectLibraryInTraceeTest, OpenUseCloseLibrary) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

  // Stop the process using our tooling.
  CHECK(AttachAndStopProcess(pid).has_value());

  // Tracee does not have the dynamic lib loaded, obviously.
  std::string kLibName = "libUserSpaceInstrumentationTestLib.so";
  auto maps_before = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  CHECK(maps_before.has_value());
  EXPECT_FALSE(absl::StrContains(maps_before.value(), kLibName));

  // Load dynamic lib into tracee.
  auto result_dlopen =
      DlopenInTracee(pid, orbit_base::GetExecutableDir() / ".." / "lib" / kLibName, RTLD_LAZY);
  CHECK(result_dlopen.has_value());

  // Tracee now does have the dynamic lib loaded.
  auto maps_after_open = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  CHECK(maps_after_open.has_value());
  EXPECT_TRUE(absl::StrContains(maps_after_open.value(), kLibName));

  // Look up symbol for "TrivialFunction".
  auto result_dlsym = DlsymInTracee(pid, result_dlopen.value(), "TrivialFunction");
  CHECK(result_dlsym.has_value());
  EXPECT_TRUE(result_dlsym.value() != nullptr);

  // Call "TrivialFunction" and assert return value.
  auto result_address_function = FindFunctionAddress(pid, "TrivialFunction", kLibName);
  CHECK(result_address_function.has_value());
  const uint64_t address_function = result_address_function.value();
  RegisterState original_registers;
  CHECK(!original_registers.BackupRegisters(pid).has_error());
  auto result_address_code = AllocateInTracee(pid, 4 * 1024);
  CHECK(result_address_code.has_value());
  const uint64_t address_code = result_address_code.value();
  // movabs rax, address_function     48 b8 address_function
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xb8})
      .AppendImmediate64(address_function)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});
  auto result_write_code = WriteTraceesMemory(pid, address_code, code.GetResultAsVector());
  CHECK(!result_write_code.has_error());

  RegisterState registers_set_rip = original_registers;
  registers_set_rip.GetGeneralPurposeRegisters()->x86_64.rip = address_code;
  CHECK(!registers_set_rip.RestoreRegisters().has_error());
  CHECK(ptrace(PTRACE_CONT, pid, 0, 0) == 0);
  CHECK(waitpid(pid, nullptr, 0) == pid);

  RegisterState return_value_registers;
  CHECK(!return_value_registers.BackupRegisters(pid).has_error());
  EXPECT_EQ(42, return_value_registers.GetGeneralPurposeRegisters()->x86_64.rax);

  CHECK(!original_registers.RestoreRegisters().has_error());
  CHECK(!FreeInTracee(pid, address_code, 4 * 1024).has_error());

  // Close the library again.
  auto result_dlclose = DlcloseInTracee(pid, result_dlopen.value());
  CHECK(!result_dlclose.has_error());

  // Now, again, the lib is absent from the tracee.
  auto maps_after_close = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  CHECK(maps_after_close.has_value());
  EXPECT_FALSE(absl::StrContains(maps_after_close.value(), kLibName));

  // Detach and end child.
  CHECK(DetachAndContinueProcess(pid).has_value());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation