// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

#include "OrbitBase/NotFoundOr.h"

namespace orbit_base {

static constexpr std::string_view kArbitraryErrorMessage{"Something went wrong"};

TEST(NotFoundOrVoid, Found) {
  NotFoundOr<void> void_or_canceled{};
  EXPECT_TRUE(void_or_canceled.HasValue());
  EXPECT_FALSE(void_or_canceled.IsNotFound());
}

TEST(NotFoundOrVoid, NotFound) {
  NotFoundOr<void> void_or_canceled{NotFound{std::string{kArbitraryErrorMessage}}};
  EXPECT_FALSE(void_or_canceled.HasValue());
  EXPECT_TRUE(void_or_canceled.IsNotFound());
  EXPECT_THAT(void_or_canceled.GetNotFound().message, kArbitraryErrorMessage);
}

TEST(NotFoundOrInt, Found) {
  NotFoundOr<int> int_or_canceled{42};
  EXPECT_TRUE(int_or_canceled.HasValue());
  EXPECT_FALSE(int_or_canceled.IsNotFound());
  EXPECT_EQ(int_or_canceled.GetValue(), 42);
  EXPECT_EQ(static_cast<const NotFoundOr<int>&>(int_or_canceled).GetValue(), 42);
  // NOLINTNEXTLINE(performance-move-const-arg)
  EXPECT_EQ(std::move(int_or_canceled).GetValue(), 42);
}

TEST(NotFoundOrInt, NotFound) {
  NotFoundOr<int> int_or_canceled{NotFound{std::string{kArbitraryErrorMessage}}};
  EXPECT_FALSE(int_or_canceled.HasValue());
  EXPECT_TRUE(int_or_canceled.IsNotFound());
  EXPECT_THAT(int_or_canceled.GetNotFound().message, kArbitraryErrorMessage);
}

TEST(NotFoundOrUniqueInt, Found) {
  NotFoundOr<std::unique_ptr<int>> unique_int_or_canceled{std::make_unique<int>(42)};
  EXPECT_TRUE(unique_int_or_canceled.HasValue());
  EXPECT_FALSE(unique_int_or_canceled.IsNotFound());
  EXPECT_EQ(*unique_int_or_canceled.GetValue(), 42);
  EXPECT_EQ(
      *static_cast<const NotFoundOr<std::unique_ptr<int>>&>(unique_int_or_canceled).GetValue(), 42);
  EXPECT_EQ(*std::move(unique_int_or_canceled).GetValue(), 42);
}

TEST(NotFoundOrUniqueInt, NotFound) {
  NotFoundOr<std::unique_ptr<int>> unique_int_or_canceled{
      NotFound{std::string{kArbitraryErrorMessage}}};
  unique_int_or_canceled = NotFound{std::string{kArbitraryErrorMessage}};
  EXPECT_FALSE(unique_int_or_canceled.HasValue());
  EXPECT_TRUE(unique_int_or_canceled.IsNotFound());
  EXPECT_THAT(unique_int_or_canceled.GetNotFound().message, kArbitraryErrorMessage);
}

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
  const std::string& message_ref{GetNotFoundMessage(not_found_or_int)};
  EXPECT_EQ(message_ref, message);

  const std::string moved_message{GetNotFoundMessage(std::move(not_found_or_int))};
  EXPECT_EQ(moved_message, message);
}

TEST(NotFoundOr, GetFound) {
  NotFoundOr<int> not_found_or_int = NotFound{"message"};
  EXPECT_DEATH(std::ignore = GetFound(not_found_or_int), "Check failed");

  not_found_or_int = 5;
  EXPECT_EQ(GetFound(not_found_or_int), 5);

  not_found_or_int = 6;
  EXPECT_EQ(GetFound(not_found_or_int), 6);
}

TEST(NotFoundOr, MoveOnlyType) {
  // unique_ptr<int>; tests move only type
  NotFoundOr<std::unique_ptr<int>> not_found_or_unique_ptr;

  EXPECT_FALSE(IsNotFound(not_found_or_unique_ptr));

  not_found_or_unique_ptr = std::make_unique<int>(5);
  EXPECT_FALSE(IsNotFound(not_found_or_unique_ptr));
  // Since no copies can be created, a reference is taken.
  const std::unique_ptr<int>& reference{GetFound(not_found_or_unique_ptr)};
  EXPECT_EQ(*reference, 5);

  not_found_or_unique_ptr = NotFound{"message"};
  ASSERT_TRUE(IsNotFound(not_found_or_unique_ptr));
  EXPECT_EQ(GetNotFoundMessage(not_found_or_unique_ptr), "message");

  // Move in and out test
  not_found_or_unique_ptr = std::make_unique<int>(5);
  const std::unique_ptr<int> moved_unique_ptr = GetFound(std::move(not_found_or_unique_ptr));
  EXPECT_EQ(*moved_unique_ptr, 5);
}

}  // namespace orbit_base