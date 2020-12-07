// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/ReadFileToString.h"

TEST(ReadFileToString, InvalidFile) {
  const auto result = orbit_base::ReadFileToString("non/existing/filename");
  ASSERT_FALSE(result);
}

TEST(ReadFileToString, Smoke) {
  const auto result = orbit_base::ReadFileToString(orbit_base::GetExecutableDir() / "testdata" /
                                                   "OrbitBase" / "textfile.txt");
  ASSERT_TRUE(result) << result.error().message();
  EXPECT_EQ(result.value(), "content\nnew line");
}
