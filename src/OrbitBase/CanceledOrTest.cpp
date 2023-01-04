// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <tuple>
#include <utility>
#include <variant>

#include "OrbitBase/CanceledOr.h"

namespace orbit_base {

TEST(CanceledOrVoid, NotCanceled) {
  CanceledOr<void> void_or_canceled{};
  EXPECT_TRUE(void_or_canceled.HasValue());
  EXPECT_FALSE(void_or_canceled.IsCanceled());
}

TEST(CanceledOrVoid, Canceled) {
  CanceledOr<void> void_or_canceled{Canceled{}};
  EXPECT_FALSE(void_or_canceled.HasValue());
  EXPECT_TRUE(void_or_canceled.IsCanceled());
}

TEST(CanceledOrInt, NotCanceled) {
  CanceledOr<int> int_or_canceled{42};
  EXPECT_TRUE(int_or_canceled.HasValue());
  EXPECT_FALSE(int_or_canceled.IsCanceled());
  EXPECT_EQ(int_or_canceled.GetValue(), 42);
  EXPECT_EQ(static_cast<const CanceledOr<int>&>(int_or_canceled).GetValue(), 42);
}

TEST(CanceledOrInt, Canceled) {
  CanceledOr<int> int_or_canceled{Canceled{}};
  EXPECT_FALSE(int_or_canceled.HasValue());
  EXPECT_TRUE(int_or_canceled.IsCanceled());
}

TEST(CanceledOrUniqueInt, NotCanceled) {
  CanceledOr<std::unique_ptr<int>> unique_int_or_canceled{std::make_unique<int>(42)};
  EXPECT_TRUE(unique_int_or_canceled.HasValue());
  EXPECT_FALSE(unique_int_or_canceled.IsCanceled());
  EXPECT_EQ(*unique_int_or_canceled.GetValue(), 42);
  EXPECT_EQ(
      *static_cast<const CanceledOr<std::unique_ptr<int>>&>(unique_int_or_canceled).GetValue(), 42);
  std::unique_ptr<int> value = std::move(unique_int_or_canceled).GetValue();
  EXPECT_THAT(value, testing::Pointee(42));
}

TEST(CanceledOrUniqueInt, Canceled) {
  CanceledOr<std::unique_ptr<int>> unique_int_or_canceled{Canceled{}};
  unique_int_or_canceled = Canceled{};
  EXPECT_FALSE(unique_int_or_canceled.HasValue());
  EXPECT_TRUE(unique_int_or_canceled.IsCanceled());
}

TEST(CanceledOr, IsCanceled) {
  // Default constructed is NOT canceled
  CanceledOr<int> canceled_or_int;
  EXPECT_FALSE(IsCanceled(canceled_or_int));

  canceled_or_int = Canceled{};
  EXPECT_TRUE(IsCanceled(canceled_or_int));

  canceled_or_int = 5;
  EXPECT_FALSE(IsCanceled(canceled_or_int));

  // Default constructed is NOT canceled
  CanceledOr<void> canceled_or_void;
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

}  // namespace orbit_base