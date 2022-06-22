// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "OrbitBase/Typedef.h"

namespace {
struct TestTag {};

}  // namespace

static int Add(int i, int j) { return i + j; }

namespace orbit_base {
template <typename T>
using Wrapper = Typedef<TestTag, T>;

TEST(TypedefTest, CanInstantiate) {
  const int kConstInt = 1;
  Wrapper<int> wrapper_of_const(kConstInt);
  EXPECT_EQ(*wrapper_of_const, kConstInt);

  constexpr int kConstexprInt = 1;
  Wrapper<int> wrapper_of_constexpr(kConstexprInt);
  EXPECT_EQ(*wrapper_of_constexpr, kConstexprInt);

  int non_const = 1;
  Wrapper<int> wrapper_of_non_const(non_const);
  EXPECT_EQ(*wrapper_of_non_const, non_const);

  Wrapper<int> wrapper_of_literal(1);
  EXPECT_EQ(*wrapper_of_literal, 1);

  Wrapper<std::string> wrapper_of_string_literal("foo");
  EXPECT_EQ(*wrapper_of_string_literal, "foo");
}

TEST(TypedefTest, CallIsCorrect) {
  const int kFirst = 1;
  const int kSecond = 2;
  const int kSum = kFirst + kSecond;

  const Wrapper<int> kFirstWrapped(kFirst);
  const Wrapper<int> kSecondWrapped(kSecond);

  {
    auto add = [](int i, int j) { return i + j; };
    const Wrapper<int> sum_wrapped = Apply(add, kFirstWrapped, kSecondWrapped);
    EXPECT_EQ(*sum_wrapped, kSum);
  }

  {
    auto add = [](const int& i, const int& j) { return i + j; };
    const Wrapper<int> sum_wrapped = Apply(add, kFirstWrapped, kSecondWrapped);
    EXPECT_EQ(*sum_wrapped, kSum);
  }

  {
    const Wrapper<int> sum_wrapped = Apply(Add, kFirstWrapped, kSecondWrapped);
    EXPECT_EQ(*sum_wrapped, kSum);
  }

  {
    bool was_called = false;
    auto returns_void = [&was_called](int /*i*/) { was_called = true; };
    const Wrapper<void> sum_wrapped = Apply(returns_void, kFirstWrapped);
    std::ignore = sum_wrapped;
    EXPECT_TRUE(was_called);
  }
}

}  // namespace orbit_base