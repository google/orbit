// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"

TEST(ExecutablePathLinux, GetExecutablePathWithPid) {
  const auto result = orbit_base::GetExecutablePath(getpid());
  ASSERT_TRUE(result) << result.error().message();
  EXPECT_EQ(result.value().filename(), "OrbitBaseTests");
}

TEST(ExecutablePathLinux, GetExecutablePathWithInvalidPid) {
  const auto result = orbit_base::GetExecutablePath(0);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().message(),
            "Unable to get executable path of process with pid 0: No such file or directory");
}

TEST(ExecutablePathLinux, GetExecutablePathWithPidPermissionDenied) {
  if (getuid() == 0) {
    GTEST_SKIP() << "This test is disabled for root user";
  }

  const auto result = orbit_base::GetExecutablePath(1);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().message(),
            "Unable to get executable path of process with pid 1: Permission denied");
}
