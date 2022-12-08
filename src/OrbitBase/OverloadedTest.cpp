// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "OrbitBase/Overloaded.h"

namespace orbit_base {

constexpr std::string_view kInt = "int";
constexpr std::string_view kString = "string";

constexpr std::string_view kTwoInts = "two ints";

constexpr auto kFromInt = [](int /*unused*/) { return kInt; };
constexpr auto kFromTwoInts = [](int /*unused*/, int /*unused*/) { return kTwoInts; };
auto kFromIntMutable = [](int /*unused*/) mutable { return kInt; };
constexpr auto kFromString = [](const std::string& /*unused*/) { return kString; };

std::string_view FromString(std::string /*unused*/) { return kString; }
std::string_view FromInt(int /*unused*/) { return kInt; }

TEST(OverloadedTest, TwoLambdas) {
  const auto overloaded_lambda = overloaded{kFromInt, kFromString};
  EXPECT_EQ(overloaded_lambda(1), kInt);
  EXPECT_EQ(overloaded_lambda("foo"), kString);
}

constexpr std::string_view kTwoStrings = "two strings";

TEST(OverloadedTest, TwoLambdasOnePolymorphic) {
  const auto overloaded_lambda =
      overloaded{[](auto /*unused*/, auto /*unused*/) { return kTwoStrings; }, kFromTwoInts};
  EXPECT_EQ(overloaded_lambda(1, 1), kTwoInts);
  EXPECT_EQ(overloaded_lambda("foo", "bar"), kTwoStrings);
}

TEST(OverloadedTest, StackedOverloaded) {
  const auto overloaded_lambda = overloaded{overloaded{kFromInt, kFromString}, kFromTwoInts};
  EXPECT_EQ(overloaded_lambda(1, 1), kTwoInts);
  EXPECT_EQ(overloaded_lambda(1), kInt);
  EXPECT_EQ(overloaded_lambda("foo"), kString);
}

TEST(OverloadedTest, MoveOnlyArgumentLambda) {
  const auto lambda = [](std::unique_ptr<int> /*unused*/) { return kInt; };
  const auto overloaded_lambda = overloaded{lambda};
  auto int_ptr = std::make_unique<int>(1);
  EXPECT_EQ(overloaded_lambda(std::move(int_ptr)), kInt);
}

TEST(OverloadedTest, TwoLambdasWithOneAndTwoArguments) {
  const auto overloaded_lambda = overloaded{kFromInt, kFromTwoInts};
  EXPECT_EQ(overloaded_lambda(1), kInt);
  EXPECT_EQ(overloaded_lambda(1, 1), kTwoInts);
}

TEST(OverloadedTest, MutableAndImmutableLambdas) {
  auto overloaded_lambda = overloaded{kFromIntMutable, kFromString};
  EXPECT_EQ(overloaded_lambda(1), kInt);
  EXPECT_EQ(overloaded_lambda("foo"), kString);
}

TEST(OverloadedTest, SingleLambda) {
  const auto overloaded_lambda = overloaded{kFromInt};
  EXPECT_EQ(overloaded_lambda(1), kInt);
}

TEST(OverloadedTest, SingleFreeFunction) {
  const auto overloaded_lambda = overloaded{&FromString};
  EXPECT_EQ(overloaded_lambda("foo"), kString);
}

TEST(OverloadedTest, TwoFreeFunctions) {
  const auto overloaded_lambda = overloaded{&FromString, &FromInt};
  EXPECT_EQ(overloaded_lambda(1), kInt);
  EXPECT_EQ(overloaded_lambda("foo"), kString);
}

TEST(OverloadedTest, FreeFunctionAndLambda) {
  const auto overloaded_lambda = overloaded{&FromString, kFromInt};
  EXPECT_EQ(overloaded_lambda(1), kInt);
  EXPECT_EQ(overloaded_lambda("foo"), kString);
}

}  // namespace orbit_base