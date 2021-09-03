// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
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

// The device ID and the inode ID together uniquely identify a file on a single machine.
// Ref: https://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html
struct DeviceInodePair {
  dev_t device;
  ino_t inode;
};

ErrorMessageOr<bool> IsDeviceInodePairInMapsFile(DeviceInodePair device_inode, pid_t pid) {
  OUTCOME_TRY(auto&& maps_contents,
              orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));

  std::vector<std::string_view> lines = absl::StrSplit(maps_contents, '\n');

  for (const auto& line : lines) {
    std::vector<std::string_view> fields = absl::StrSplit(line, ' ', absl::SkipEmpty{});
    constexpr int kInodeFieldIndex = 4;
    if (fields.size() <= kInodeFieldIndex) continue;

    ino_t current_inode{};
    if (!absl::SimpleAtoi(fields[kInodeFieldIndex], &current_inode)) continue;
    if (current_inode != device_inode.inode) continue;

    // The device field is formatted as `AA:BB` where `AA` is a single hex byte representing the
    // major number and `BB` being a single hex byte representing the minor number. Both numbers
    // are always two digits wide.
    constexpr int kDeviceFieldIndex = 3;
    std::vector<std::string_view> device_number_fields =
        absl::StrSplit(fields[kDeviceFieldIndex], ':');
    if (device_number_fields.size() != 2) continue;

    constexpr int kMajorFieldIndex = 0;
    if (device_number_fields[kMajorFieldIndex].size() != 2) continue;
    uint32_t major_number{};
    if (std::from_chars(device_number_fields[kMajorFieldIndex].begin(),
                        device_number_fields[kMajorFieldIndex].end(), major_number, 16)
            .ec != std::errc{}) {
      continue;
    }

    constexpr int kMinorFieldIndex = 1;
    if (device_number_fields[kMinorFieldIndex].size() != 2) continue;
    uint32_t minor_number{};
    if (std::from_chars(device_number_fields[kMinorFieldIndex].begin(),
                        device_number_fields[kMinorFieldIndex].end(), minor_number, 16)
            .ec != std::errc{}) {
      continue;
    }

    if (makedev(major_number, minor_number) == device_inode.device) return true;
  }

  return false;
}

ErrorMessageOr<DeviceInodePair> GetDeviceInodePairFromFilePath(const std::string& file_path) {
  struct stat stat_buf {};
  if (stat(file_path.c_str(), &stat_buf) != 0) {
    return ErrorMessage{absl::StrFormat("Failed to obtain inode of '%s'", file_path)};
  }
  return DeviceInodePair{stat_buf.st_dev, stat_buf.st_ino};
}

void OpenUseAndCloseLibrary(pid_t pid) {
  // Stop the child process using our tooling.
  CHECK(!AttachAndStopProcess(pid).has_error());

  auto library_path_or_error = GetTestLibLibraryPath();
  ASSERT_THAT(library_path_or_error, HasNoError());
  std::filesystem::path library_path = std::move(library_path_or_error.value());

  ErrorMessageOr<DeviceInodePair> device_inode_pair_of_library =
      GetDeviceInodePairFromFilePath(library_path);
  ASSERT_THAT(device_inode_pair_of_library, HasNoError());

  // Tracee does not have the dynamic lib loaded, obviously.
  EXPECT_THAT(IsDeviceInodePairInMapsFile(device_inode_pair_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(false)));

  auto library_handle_or_error = DlopenInTracee(pid, library_path, RTLD_NOW);
  ASSERT_TRUE(library_handle_or_error.has_value());

  // Tracee now does have the dynamic lib loaded.
  EXPECT_THAT(IsDeviceInodePairInMapsFile(device_inode_pair_of_library.value(), pid),
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
  EXPECT_THAT(IsDeviceInodePairInMapsFile(device_inode_pair_of_library.value(), pid),
              orbit_test_utils::HasValue(Eq(false)));

  CHECK(!DetachAndContinueProcess(pid).has_error());
}

}  // namespace

TEST(InjectLibraryInTraceeTest, OpenUseAndCloseLibraryInUserCode) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
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
