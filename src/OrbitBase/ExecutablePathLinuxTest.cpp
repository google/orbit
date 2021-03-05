// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <unistd.h>

#include <filesystem>
#include <memory>
#include <outcome.hpp>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Result.h"

TEST(ExecutablePathLinux, GetExecutablePathWithPid) {
  const auto path_or_error = orbit_base::GetExecutablePath(getpid());
  ASSERT_FALSE(path_or_error.has_error()) << path_or_error.error().message();
  EXPECT_EQ(path_or_error.value().filename(), "OrbitBaseTests");
}

TEST(ExecutablePathLinux, GetExecutablePathWithInvalidPid) {
  const auto path_or_error = orbit_base::GetExecutablePath(0);
  ASSERT_TRUE(path_or_error.has_error());
  EXPECT_EQ(path_or_error.error().message(),
            "Unable to get executable path of process with pid 0: No such file or directory");
}
