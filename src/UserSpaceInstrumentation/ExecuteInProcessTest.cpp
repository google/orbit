// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/wait.h>

#include <csignal>
#include <string>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

using orbit_base::HasNoError;
using orbit_base::HasValue;

TEST(ExecuteInProcessTest, ExecuteInProcess) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

  CHECK(!AttachAndStopProcess(pid).has_error());

  // Load dynamic lib into tracee.
  const std::string kLibName = "libUserSpaceInstrumentationTestLib.so";
  const std::string library_path = orbit_base::GetExecutableDir() / ".." / "lib" / kLibName;
  auto library_handle_or_error = DlopenInTracee(pid, library_path, RTLD_NOW);
  CHECK(library_handle_or_error.has_value());
  void* library_handle = library_handle_or_error.value();

  auto result_or_error = ExecuteInProcess(pid, library_handle, "TrivialFunction");
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_EQ(42, result_or_error.value());

  result_or_error = ExecuteInProcess(pid, library_handle, "TrivialSum", 2, 4, 6, 8, 10, 12);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_EQ(42, result_or_error.value());

  auto function_address_or_error = DlsymInTracee(pid, library_handle, "TrivialFunction");
  ASSERT_THAT(result_or_error, HasValue());
  result_or_error = ExecuteInProcess(pid, function_address_or_error.value());
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(42, result_or_error.value());

  function_address_or_error = DlsymInTracee(pid, library_handle, "TrivialSum");
  ASSERT_TRUE(function_address_or_error.has_value());
  result_or_error = ExecuteInProcess(pid, function_address_or_error.value(), 2, 4, 6, 8, 10, 12);
  ASSERT_THAT(result_or_error, HasValue());
  EXPECT_EQ(42, result_or_error.value());

  // Cleanup, detach and end child.
  CHECK(!DlcloseInTracee(pid, library_handle).has_error());
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation