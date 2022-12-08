// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <dlfcn.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "GetTestLibLibraryPath.h"
#include "GrpcProtos/module.pb.h"
#include "MachineCode.h"
#include "ModuleUtils/ReadLinuxModules.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using testing::Eq;

// This function checks if a certain inode appears in the maps file
// of the process with PID `pid`. It is only comparing the inode and
// not the device id because the latter has proven to be unrealiable
// when using overlayfs. That's the case on the CI because Docker uses
// overlayfs. Fixing that properly is a non-trivial task and isn't
// justified compared to the risk of having an inode clash.
ErrorMessageOr<bool> IsInodeInMapsFile(ino_t inode, pid_t pid) {
  OUTCOME_TRY(auto&& maps_contents,
              orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));

  std::vector<std::string_view> lines = absl::StrSplit(maps_contents, '\n');

  for (const auto& line : lines) {
    std::vector<std::string_view> fields = absl::StrSplit(line, ' ', absl::SkipEmpty{});
    constexpr int kInodeFieldIndex = 4;
    if (fields.size() <= kInodeFieldIndex) continue;

    ino_t current_inode{};
    if (!absl::SimpleAtoi(fields[kInodeFieldIndex], &current_inode)) continue;
    if (current_inode == inode) return true;
  }

  return false;
}

ErrorMessageOr<ino_t> GetInodeFromFilePath(const std::filesystem::path& file_path) {
  struct stat stat_buf {};
  if (stat(file_path.string().c_str(), &stat_buf) != 0) {
    return ErrorMessage{absl::StrFormat("Failed to obtain inode of '%s'", file_path)};
  }
  return stat_buf.st_ino;
}

void OpenUseAndCloseLibrary(pid_t pid) {
  // Stop the child process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  auto library_path_or_error = GetTestLibLibraryPath();
  ASSERT_THAT(library_path_or_error, HasNoError());
  std::filesystem::path library_path = std::move(library_path_or_error.value());

  ErrorMessageOr<ino_t> inode_of_library = GetInodeFromFilePath(library_path.string());
  ASSERT_THAT(inode_of_library, HasNoError());

  // Tracee does not have the dynamic lib loaded, obviously.
  EXPECT_THAT(IsInodeInMapsFile(inode_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(false)));

  auto modules_or_error = orbit_module_utils::ReadModules(pid);
  ASSERT_THAT(modules_or_error, orbit_test_utils::HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo>& modules = modules_or_error.value();

  auto library_handle_or_error =
      DlmopenInTracee(pid, modules, library_path, RTLD_NOW, LinkerNamespace::kCreateNewNamespace);
  ASSERT_TRUE(library_handle_or_error.has_value());

  // Tracee now does have the dynamic lib loaded.
  EXPECT_THAT(IsInodeInMapsFile(inode_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(true)));

  // Look up symbol for "TrivialFunction" in the dynamic lib.
  auto dlsym_or_error =
      DlsymInTracee(pid, modules, library_handle_or_error.value(), "TrivialFunction");
  ASSERT_TRUE(dlsym_or_error.has_value());
  const auto function_address = absl::bit_cast<uint64_t>(dlsym_or_error.value());

  {
    // Write machine code to call "TrivialFunction" from the dynamic lib.
    constexpr uint64_t kScratchPadSize = 1024;
    auto memory_or_error = AutomaticMemoryInTracee::Create(pid, 0, kScratchPadSize);
    ASSERT_TRUE(memory_or_error.has_value());
    // Move function's address to rax, do the call, and hit a breakpoint:
    // movabs rax, function_address     48 b8 function_address
    // call rax                         ff d0
    // int3                             cc
    MachineCode code;
    code.AppendBytes({0x48, 0xb8})
        .AppendImmediate64(function_address)
        .AppendBytes({0xff, 0xd0})
        .AppendBytes({0xcc});

    auto result_or_error = ExecuteMachineCode(*memory_or_error.value(), code);
    ASSERT_THAT(result_or_error, HasNoError());
    EXPECT_EQ(42, result_or_error.value());
  }

  // Close the library.
  auto result_dlclose = DlcloseInTracee(pid, modules, library_handle_or_error.value());
  ASSERT_THAT(result_dlclose, HasNoError());

  // Now, again, the lib is absent from the tracee.
  EXPECT_THAT(IsInodeInMapsFile(inode_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(false)));

  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());
}

}  // namespace

TEST(InjectLibraryInTraceeTest, OpenUseAndCloseLibraryInUserCode) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  OpenUseAndCloseLibrary(pid);

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(InjectLibraryInTraceeTest, OpenUseAndCloseLibraryInSyscall) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    while (true) {
      // Child will be stuck in syscall sys_clock_nanosleep.
      std::this_thread::sleep_for(std::chrono::hours(1000000000));
    }
  }

  OpenUseAndCloseLibrary(pid);

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(InjectLibraryInTraceeTest, NonExistingLibrary) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    while (true) {
      // Child will be stuck in syscall sys_clock_nanosleep.
      std::this_thread::sleep_for(std::chrono::hours(1000000000));
    }
  }

  // Stop the child process using our tooling.
  ORBIT_CHECK(!AttachAndStopProcess(pid).has_error());

  auto modules_or_error = orbit_module_utils::ReadModules(pid);
  ASSERT_THAT(modules_or_error, orbit_test_utils::HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo>& modules = modules_or_error.value();

  // Try to load non existing dynamic lib into tracee.
  const std::string non_existing_lib_name = "libNotFound.so";
  auto library_handle_or_error = DlmopenInTracee(pid, modules, non_existing_lib_name, RTLD_NOW,
                                                 LinkerNamespace::kCreateNewNamespace);
  ASSERT_THAT(library_handle_or_error, HasError("Library does not exist at"));

  // Continue child process.
  ORBIT_CHECK(!DetachAndContinueProcess(pid).has_error());

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation
