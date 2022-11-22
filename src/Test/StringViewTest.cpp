// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string_view>

namespace orbit_test {

// Ensures that GTest was compiled with std::string_view support.
TEST(Test, MatchersSupportStringView) {
  std::string_view some_string = "Hello World!";
  EXPECT_THAT(some_string, testing::HasSubstr("World!"));
}
}  // namespace orbit_test
