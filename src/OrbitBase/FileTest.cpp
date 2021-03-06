// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"

namespace orbit_base {

TEST(File, DefaultUniqueFdIsInvalidDescriptor) {
  unique_fd fd;
  EXPECT_FALSE(fd.valid());
  EXPECT_EQ(fd.get(), kInvalidFd);
}

TEST(File, EmptyUnuqueFdCanBeReleased) {
  unique_fd fd;
  fd.release();
  EXPECT_FALSE(fd.valid());
  EXPECT_EQ(fd.get(), kInvalidFd);
}

TEST(File, MoveAssingToExisingUniqueFd) {
  unique_fd fd;

  auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");

  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
  fd = std::move(fd_or_error.value());
  EXPECT_TRUE(fd.valid());
  EXPECT_FALSE(fd_or_error.value().valid());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"

TEST(File, UniqueFdSelfMove) {
  auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");
  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
  unique_fd valid_fd{std::move(fd_or_error.value())};

  valid_fd = std::move(valid_fd);

  ASSERT_TRUE(valid_fd.valid());
}

#pragma GCC diagnostic pop

TEST(File, OpenFileForReadingInvalidFile) {
  const auto fd_or_error = OpenFileForReading("non/existing/filename");
  ASSERT_TRUE(fd_or_error.has_error());
}

TEST(File, ReadFullySmoke) {
  const auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");
  ASSERT_FALSE(fd_or_error.has_error()) << fd_or_error.error().message();
  const auto& fd = fd_or_error.value();
  ASSERT_TRUE(fd.valid());
  std::array<char, 64> buf{};

  ErrorMessageOr<size_t> result = ReadFully(fd, buf.data(), 5);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 5);
  EXPECT_STREQ(buf.data(), "conte");

  buf = {};

  result = ReadFully(fd, buf.data(), buf.size());
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 11);
  EXPECT_STREQ(buf.data(), "nt\nnew line");

  buf = {};

  result = ReadFully(fd, buf.data(), buf.size());
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 0);
  EXPECT_STREQ(buf.data(), "");
}

}  // namespace orbit_base