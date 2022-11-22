// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/AnyMovable.h"

namespace orbit_base {

TEST(AnyMovable, DefaultConstruction) {
  AnyMovable any{};
  EXPECT_FALSE(any.HasValue());
}

TEST(AnyMovable, CarryInt) {
  AnyMovable any{42};

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(42));
}

TEST(AnyMovable, CarryUniquePtr) {
  AnyMovable any{std::make_unique<int>(42)};

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(std::unique_ptr<int>));
}

TEST(AnyMovable, InPlaceConstructInt) {
  AnyMovable any{std::in_place_type<int>, 42};

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(int));
}

TEST(AnyMovable, InPlaceConstructUniquePtr) {
  AnyMovable any{std::in_place_type<std::unique_ptr<int>>, new int{42}};

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(std::unique_ptr<int>));
}

TEST(AnyMovable, EmplaceInt) {
  AnyMovable any{};
  any.Emplace<int>(42);

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(int));
}

TEST(AnyMovable, EmplaceUniquePtr) {
  AnyMovable any{};
  any.Emplace<std::unique_ptr<int>>(new int{42});

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(std::unique_ptr<int>));
}

TEST(any_movable_cast, ExtractInt) {
  AnyMovable any{42};

  auto* ptr = any_movable_cast<int>(&any);
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(*ptr, 42);
}
TEST(any_movable_cast, ExtractUniquePtr) {
  AnyMovable any{std::make_unique<int>(42)};

  auto* ptr = any_movable_cast<std::unique_ptr<int>>(&any);
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(**ptr, 42);
}

TEST(any_movable_cast, RefuseExtractingWrongType) {
  AnyMovable any{std::make_unique<int>(42)};

  auto* ptr = any_movable_cast<int>(&any);
  EXPECT_EQ(ptr, nullptr);
}

TEST(MakeAnyMovable, InPlaceConstructInt) {
  auto any = MakeAnyMovable<int>(42);

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(int));
}

TEST(MakeAnyMovable, InPlaceConstructUniquePtr) {
  auto any = MakeAnyMovable<std::unique_ptr<int>>(new int{42});

  EXPECT_TRUE(any.HasValue());
  EXPECT_EQ(any.type(), typeid(std::unique_ptr<int>));
}

}  // namespace orbit_base