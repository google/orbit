// TODO(b/148520406): Add copyright here
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Path.h"
#include "absl/strings/match.h"

TEST(Path, GetSourceRoot) {
  // On windows GetSroucePath converts '\' to '/'
  // to account for that call GetDirectory
  ASSERT_TRUE(
      absl::StartsWith(Path::GetDirectory(__FILE__), Path::GetSourceRoot()));
}

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
