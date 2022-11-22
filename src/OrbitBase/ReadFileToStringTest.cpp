// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"

TEST(ReadFileToString, InvalidFile) {
  const auto fd_or_error = orbit_base::ReadFileToString("non/existing/filename");
  ASSERT_TRUE(fd_or_error.has_error());
}

TEST(ReadFileToString, Smoke) {
  const auto fd_or_error =
      orbit_base::ReadFileToString(orbit_test::GetTestdataDir() / "textfile.bin");
  ASSERT_FALSE(fd_or_error.has_error()) << fd_or_error.error().message();
  EXPECT_EQ(fd_or_error.value(), "content\nnew line");
}
