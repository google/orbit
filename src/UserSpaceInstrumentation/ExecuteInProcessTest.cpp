// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <csignal>
#include <filesystem>
#include <string>

#include "GetTestLibLibraryPath.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;

namespace {

class ExecuteInProcessTest : public testing::Test {
 protected:
  ExecuteInProcessTest() {
    pid_ = fork();
    CHECK(pid_ != -1);
    if (pid_ == 0) {
      prctl(PR_SET_PDEATHSIG, SIGTERM);

      volatile uint64_t counter = 0;
      while (true) {
        // Endless loops without side effects are UB and recent versions of clang optimize it away.
        ++counter;
      }
    }

    CHECK(!AttachAndStopProcess(pid_).has_error());

    auto library_path_or_error = GetTestLibLibraryPath();
    CHECK(library_path_or_error.has_value());
    std::filesystem::path library_path = std::move(library_path_or_error.value());

    // Load dynamic lib into tracee.
    auto library_handle_or_error = DlopenInTracee(pid_, library_path, RTLD_NOW);
    CHECK(library_handle_or_error.has_value());
    library_handle_ = library_handle_or_error.value();
  }

  ~ExecuteInProcessTest() override {
    // Cleanup, detach and end child.
    CHECK(!DlcloseInTracee(pid_, library_handle_).has_error());
    CHECK(!DetachAndContinueProcess(pid_).has_error());
    kill(pid_, SIGKILL);
    waitpid(pid_, nullptr, 0);
  }

  pid_t pid_ = 0;
  void* library_handle_ = nullptr;
};

}  // namespace

TEST_F(ExecuteInProcessTest, ExecuteInProcess) {
  auto result_or_error = ExecuteInProcess(pid_, library_handle_, "TrivialFunction");
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_EQ(42, result_or_error.value());

  result_or_error = ExecuteInProcess(pid_, library_handle_, "TrivialSum", 2, 4, 6, 8, 10, 12);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_EQ(42, result_or_error.value());

  auto function_address_or_error = DlsymInTracee(pid_, library_handle_, "TrivialFunction");
  ASSERT_THAT(result_or_error, HasValue());
  result_or_error = ExecuteInProcess(pid_, function_address_or_error.value());
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(42, result_or_error.value());

  function_address_or_error = DlsymInTracee(pid_, library_handle_, "TrivialSum");
  ASSERT_TRUE(function_address_or_error.has_value());
  result_or_error = ExecuteInProcess(pid_, function_address_or_error.value(), 2, 4, 6, 8, 10, 12);
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(42, result_or_error.value());
}

TEST_F(ExecuteInProcessTest, ExecuteInProcessWithMicrosoftCallingConvention) {
  auto function_address_or_error = DlsymInTracee(pid_, library_handle_, "TrivialSumWithMsAbi");
  ASSERT_TRUE(function_address_or_error.has_value());
  auto result_or_error = ExecuteInProcessWithMicrosoftCallingConvention(
      pid_, function_address_or_error.value(), 2, 4, 6, 8);
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(20, result_or_error.value());
}

}  // namespace orbit_user_space_instrumentation