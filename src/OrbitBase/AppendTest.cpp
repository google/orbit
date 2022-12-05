// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "OrbitBase/Append.h"

namespace orbit_base {

TEST(Append, SourceIsVectorVariable) {
  std::vector<std::string> dest{"a", "b"};
  std::vector<std::string> source{"c", "d", "e"};
  Append(dest, source);
  EXPECT_THAT(dest, ::testing::ElementsAre("a", "b", "c", "d", "e"));
}

TEST(Append, SourceIsTemporaryVector) {
  std::vector<std::string> dest{"a", "b"};
  Append(dest, std::vector<std::string>{"c", "d", "e"});
  EXPECT_THAT(dest, ::testing::ElementsAre("a", "b", "c", "d", "e"));
}

TEST(Append, SourceIsInitializerList) {
  std::vector<std::string> dest{"a", "b"};
  Append(dest, {"c", "d", "e"});
  EXPECT_THAT(dest, ::testing::ElementsAre("a", "b", "c", "d", "e"));
}

}  // namespace orbit_base