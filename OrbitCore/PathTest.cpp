// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Path.h"
#include "absl/strings/match.h"

TEST(Path, FileExistsEmptyFilename) {
  std::string filename{};
  EXPECT_FALSE(Path::FileExists(filename));
}

TEST(Path, FileExistsRootDir) {
  std::string filename;
#ifdef _WIN32
  filename = "C:\\";
#else
  filename = "/";
#endif
  EXPECT_TRUE(Path::FileExists(filename));
}

#ifndef _WIN32
TEST(Path, FileExistsDevNull) {
  std::string filename = "/dev/null";
  EXPECT_TRUE(Path::FileExists(filename));
}
#endif

TEST(Path, JoinPathPartsEmpty) {
  std::string actual = Path::JoinPath({});
  std::string expected{};
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsRelativeSlash) {
  std::string actual = Path::JoinPath({"dir1/dir2", "dir3/dir4/", "dir5/dir6"});
#ifdef _WIN32
  std::string expected = "dir1/dir2\\dir3/dir4/dir5/dir6";
#else
  std::string expected = "dir1/dir2/dir3/dir4/dir5/dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteSlash) {
  std::string actual =
      Path::JoinPath({"/dir1/dir2", "dir3/dir4/", "dir5/dir6"});
#ifdef _WIN32
  std::string expected = "/dir1/dir2\\dir3/dir4/dir5/dir6";
#else
  std::string expected = "/dir1/dir2/dir3/dir4/dir5/dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteRootSlash) {
  std::string actual = Path::JoinPath({"/", "dir1/dir2", "dir3/dir4"});
#ifdef _WIN32
  std::string expected = "/dir1/dir2\\dir3/dir4";
#else
  std::string expected = "/dir1/dir2/dir3/dir4";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsRelativeBackslash) {
  std::string actual =
      Path::JoinPath({"dir1\\dir2", "dir3\\dir4\\", "dir5\\dir6"});
#ifdef _WIN32
  std::string expected = "dir1\\dir2\\dir3\\dir4\\dir5\\dir6";
#else
  std::string expected = "dir1\\dir2/dir3\\dir4\\/dir5\\dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteBackslash) {
  std::string actual =
      Path::JoinPath({"C:\\dir1\\dir2", "dir3\\dir4\\", "dir5\\dir6"});
#ifdef _WIN32
  std::string expected = "C:\\dir1\\dir2\\dir3\\dir4\\dir5\\dir6";
#else
  std::string expected = "C:\\dir1\\dir2/dir3\\dir4\\/dir5\\dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteRootBackslash) {
  std::string actual = Path::JoinPath({"C:", "dir1\\dir2", "dir3\\dir4"});
#ifdef _WIN32
  std::string expected = "C:dir1\\dir2\\dir3\\dir4";
#else
  std::string expected = "C:/dir1\\dir2/dir3\\dir4";
#endif
  EXPECT_EQ(actual, expected);
}
