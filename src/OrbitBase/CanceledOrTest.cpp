// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "OrbitBase/CanceledOr.h"
#include "outcome.hpp"

namespace orbit_base {

TEST(CanceledOr, IsCanceled) {
  CanceledOr<int> canceled_or_int{0};
  EXPECT_FALSE(IsCanceled(canceled_or_int));

  canceled_or_int = Canceled{};
  EXPECT_TRUE(IsCanceled(canceled_or_int));

  canceled_or_int = 5;
  EXPECT_FALSE(IsCanceled(canceled_or_int));

  CanceledOr<void> canceled_or_void{outcome::success()};
  EXPECT_FALSE(IsCanceled(canceled_or_void));

  canceled_or_void = Canceled{};
  EXPECT_TRUE(IsCanceled(canceled_or_void));
}

TEST(CanceledOr, GetNotCanceled) {
  CanceledOr<int> canceled_or_int{Canceled{}};
  EXPECT_DEATH(std::ignore = GetNotCanceled(canceled_or_int), "Check failed");

  canceled_or_int = 5;
  EXPECT_EQ(GetNotCanceled(canceled_or_int), 5);
}

TEST(CanceledOr, GetNotCanceledMoveOnly) {
  CanceledOr<std::unique_ptr<int>> canceled_or_unique_ptr{Canceled{}};
  EXPECT_DEATH(std::ignore = GetNotCanceled(canceled_or_unique_ptr), "Check failed");

  canceled_or_unique_ptr = std::make_unique<int>(5);
  const std::unique_ptr<int>& reference{GetNotCanceled(canceled_or_unique_ptr)};
  EXPECT_EQ(*reference, 5);

  std::unique_ptr<int> moved_unique_ptr{GetNotCanceled(std::move(canceled_or_unique_ptr))};
  EXPECT_EQ(*moved_unique_ptr, 5);
}

TEST(Canceled, GetMessage) {
  // We test whether the return type of `Canceled::message()` can be casted to a `std::string`
  // (compile time check) and whether it returns some non-empty string (runtime check). There is no
  // point in checking the actual string, as this would just duplicate the static string and does
  // not add anything in terms of test coverage.
  EXPECT_THAT(std::string{Canceled{}.message()}, testing::Not(testing::IsEmpty()));
}

}  // namespace orbit_base