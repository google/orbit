// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>

#include <charconv>
#include <chrono>
#include <csignal>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>

#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "GetTestLibLibraryPath.h"
#include "MachineCode.h"
#include "OrbitBase/ExecutablePath.h"
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

ErrorMessageOr<ino_t> GetInodeFromFilePath(const std::string& file_path) {
  struct stat stat_buf {};
  if (stat(file_path.c_str(), &stat_buf) != 0) {
    return ErrorMessage{absl::StrFormat("Failed to obtain inode of '%s'", file_path)};
  }
  return stat_buf.st_ino;
}

void OpenUseAndCloseLibrary(pid_t pid) {
  // Stop the child process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  auto library_path_or_error = GetTestLibLibraryPath();
  ASSERT_THAT(library_path_or_error, HasNoError());
  std::filesystem::path library_path = std::move(library_path_or_error.value());

  ErrorMessageOr<ino_t> inode_of_library = GetInodeFromFilePath(library_path);
  ASSERT_THAT(inode_of_library, HasNoError());

  // Tracee does not have the dynamic lib loaded, obviously.
  EXPECT_THAT(IsInodeInMapsFile(inode_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(false)));

  auto library_handle_or_error = DlopenInTracee(pid, library_path, RTLD_NOW);
  ASSERT_TRUE(library_handle_or_error.has_value());

  // Tracee now does have the dynamic lib loaded.
  EXPECT_THAT(IsInodeInMapsFile(inode_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(true)));

  // Look up symbol for "TrivialFunction" in the dynamic lib.
  auto dlsym_or_error = DlsymInTracee(pid, library_handle_or_error.value(), "TrivialFunction");
  ASSERT_TRUE(dlsym_or_error.has_value());
  const uint64_t function_address = absl::bit_cast<uint64_t>(dlsym_or_error.value());

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
  auto result_dlclose = DlcloseInTracee(pid, library_handle_or_error.value());
  ASSERT_THAT(result_dlclose, HasNoError());

  // Now, again, the lib is absent from the tracee.
  EXPECT_THAT(IsInodeInMapsFile(inode_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(false)));

  CHECK(!DetachAndContinueProcess(pid).has_error());
}

}  // namespace

TEST(InjectLibraryInTraceeTest, OpenUseAndCloseLibraryInUserCode) {
  pid_t pid = fork();
  CHECK(pid != -1);
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
  pid_t pid = fork();
  CHECK(pid != -1);
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
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    while (true) {
      // Child will be stuck in syscall sys_clock_nanosleep.
      std::this_thread::sleep_for(std::chrono::hours(1000000000));
    }
  }

  // Stop the child process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  // Try to load non existing dynamic lib into tracee.
  const std::string kNonExistingLibName = "libNotFound.so";
  auto library_handle_or_error = DlopenInTracee(pid, kNonExistingLibName, RTLD_NOW);
  ASSERT_THAT(library_handle_or_error, HasError("Library does not exist at"));

  // Continue child process.
  CHECK(!DetachAndContinueProcess(pid).has_error());

  // End child process.
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation
