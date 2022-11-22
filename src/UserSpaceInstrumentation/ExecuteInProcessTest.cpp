// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dlfcn.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <filesystem>
#include <utility>
#include <vector>

#include "GetTestLibLibraryPath.h"
#include "GrpcProtos/module.pb.h"
#include "ModuleUtils/ReadLinuxModules.h"
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
  void StartAndAttach() {
    pid_ = fork();
    ORBIT_CHECK(pid_ != -1);
    if (pid_ == 0) {
      prctl(PR_SET_PDEATHSIG, SIGTERM);

      volatile uint64_t counter = 0;
      while (true) {
        // Endless loops without side effects are UB and recent versions of clang optimize it away.
        ++counter;
      }
    }

    ORBIT_CHECK(!AttachAndStopProcess(pid_).has_error());

    auto library_path_or_error = GetTestLibLibraryPath();
    ORBIT_CHECK(library_path_or_error.has_value());
    std::filesystem::path library_path = std::move(library_path_or_error.value());

    // Load dynamic lib into tracee.
    auto modules_or_error = orbit_module_utils::ReadModules(pid_);
    ORBIT_CHECK(modules_or_error.has_value());
    auto library_handle_or_error = DlmopenInTracee(pid_, modules_or_error.value(), library_path,
                                                   RTLD_NOW, LinkerNamespace::kCreateNewNamespace);
    ORBIT_CHECK(library_handle_or_error.has_value());
    library_handle_ = library_handle_or_error.value();
  }

  void DetachAndStop() {
    // Cleanup, detach and end child.
    auto modules_or_error = orbit_module_utils::ReadModules(pid_);
    ORBIT_CHECK(modules_or_error.has_value());
    ORBIT_CHECK(!DlcloseInTracee(pid_, modules_or_error.value(), library_handle_).has_error());
    ORBIT_CHECK(!DetachAndContinueProcess(pid_).has_error());
    kill(pid_, SIGKILL);
    waitpid(pid_, nullptr, 0);
  }

  pid_t pid_ = 0;
  void* library_handle_ = nullptr;
};

}  // namespace

TEST_F(ExecuteInProcessTest, ExecuteInProcess) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  StartAndAttach();

  auto modules_or_error = orbit_module_utils::ReadModules(pid_);
  ASSERT_THAT(modules_or_error, orbit_test_utils::HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo>& modules = modules_or_error.value();

  auto result_or_error = ExecuteInProcess(pid_, modules, library_handle_, "TrivialFunction");
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_EQ(42, result_or_error.value());

  result_or_error =
      ExecuteInProcess(pid_, modules, library_handle_, "TrivialSum", 2, 4, 6, 8, 10, 12);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_EQ(42, result_or_error.value());

  auto function_address_or_error = DlsymInTracee(pid_, modules, library_handle_, "TrivialFunction");
  ASSERT_THAT(result_or_error, HasValue());
  result_or_error = ExecuteInProcess(pid_, function_address_or_error.value());
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(42, result_or_error.value());

  function_address_or_error = DlsymInTracee(pid_, modules, library_handle_, "TrivialSum");
  ASSERT_TRUE(function_address_or_error.has_value());
  result_or_error = ExecuteInProcess(pid_, function_address_or_error.value(), 2, 4, 6, 8, 10, 12);
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(42, result_or_error.value());

  DetachAndStop();
}

TEST_F(ExecuteInProcessTest, ExecuteInProcessWithMicrosoftCallingConvention) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  StartAndAttach();

  auto modules_or_error = orbit_module_utils::ReadModules(pid_);
  ASSERT_THAT(modules_or_error, orbit_test_utils::HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo>& modules = modules_or_error.value();

  auto function_address_or_error =
      DlsymInTracee(pid_, modules, library_handle_, "TrivialSumWithMsAbi");
  ASSERT_TRUE(function_address_or_error.has_value());
  auto result_or_error = ExecuteInProcessWithMicrosoftCallingConvention(
      pid_, function_address_or_error.value(), 2, 4, 6, 8);
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(20, result_or_error.value());

  DetachAndStop();
}

}  // namespace orbit_user_space_instrumentation