// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "OrbitBase/NotFoundOr.h"

namespace orbit_base {

TEST(NotFoundOr, IsNotFound) {
  // Default constructor is found
  NotFoundOr<int> not_found_or_int;
  EXPECT_FALSE(IsNotFound(not_found_or_int));

  not_found_or_int = NotFound{"message"};
  EXPECT_TRUE(IsNotFound(not_found_or_int));

  not_found_or_int = 5;
  EXPECT_FALSE(IsNotFound(not_found_or_int));

  NotFoundOr<void> not_found_or_void;
  EXPECT_FALSE(IsNotFound(not_found_or_void));

  not_found_or_void = NotFound{"message"};
  EXPECT_TRUE(IsNotFound(not_found_or_void));
}

TEST(NotFoundOr, GetNotFoundMessage) {
  NotFoundOr<int> not_found_or_int;
  EXPECT_DEATH(std::ignore = GetNotFoundMessage(not_found_or_int), "Check failed");

  not_found_or_int = 5;
  EXPECT_DEATH(std::ignore = GetNotFoundMessage(not_found_or_int), "Check failed");

  const std::string message{"example message"};
  not_found_or_int = NotFound{message};
  EXPECT_EQ(GetNotFoundMessage(not_found_or_int), message);
}

TEST(NotFoundOr, MoveOnlyType) {
  // unique_ptr<int>; tests move only type
  NotFoundOr<std::unique_ptr<int>> not_found_or_unique_ptr;

  EXPECT_FALSE(IsNotFound(not_found_or_unique_ptr));

  not_found_or_unique_ptr = std::make_unique<int>(5);
  EXPECT_FALSE(IsNotFound(not_found_or_unique_ptr));

  not_found_or_unique_ptr = NotFound{"message"};
  ASSERT_TRUE(IsNotFound(not_found_or_unique_ptr));
  EXPECT_EQ(GetNotFoundMessage(not_found_or_unique_ptr), "message");
}

}  // namespace orbit_base