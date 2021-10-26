// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <unistd.h>

#include <filesystem>
#include <memory>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"

TEST(ExecutablePathLinux, GetExecutablePathWithPid) {
  /* copybara:insert(executable is named differently)
  GTEST_SKIP();
  */

  const auto path_or_error =
      orbit_base::GetExecutablePath(orbit_base::FromNativeProcessId(getpid()));
  ASSERT_FALSE(path_or_error.has_error()) << path_or_error.error().message();
  EXPECT_EQ(path_or_error.value().filename(), "OrbitBaseTests");
}

TEST(ExecutablePathLinux, GetExecutablePathWithInvalidPid) {
  const auto path_or_error = orbit_base::GetExecutablePath(0);
  ASSERT_TRUE(path_or_error.has_error());
  EXPECT_EQ(path_or_error.error().message(),
            "Unable to get executable path of process with pid 0: No such file or directory");
}
