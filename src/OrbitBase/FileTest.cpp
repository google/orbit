// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"

TEST(File, OpenFileForReadingInvalidFile) {
  const auto result = orbit_base::OpenFileForReading("non/existing/filename");
  ASSERT_TRUE(result.has_error());
}

TEST(File, ReadFullySmoke) {
  const auto open_result = orbit_base::OpenFileForReading(
      orbit_base::GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");
  ASSERT_FALSE(open_result.has_error()) << open_result.error().message();
  const auto& fd = open_result.value();
  std::array<char, 64> buf{};

  ErrorMessageOr<size_t> result = orbit_base::ReadFully(fd, buf.data(), 5);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 5);
  EXPECT_STREQ(buf.data(), "conte");

  buf = {};

  result = orbit_base::ReadFully(fd, buf.data(), buf.size());
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 11);
  EXPECT_STREQ(buf.data(), "nt\nnew line");

  buf = {};

  result = orbit_base::ReadFully(fd, buf.data(), buf.size());
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 0);
  EXPECT_STREQ(buf.data(), "");
}
