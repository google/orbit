// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <mutex>
#include <tuple>
#include <utility>
#include <vector>

#include "OrbitBase/Typedef.h"

namespace {
struct TestTag {};

struct Integer {
  [[nodiscard]] Integer Add(const Integer& other) const { return {value + other.value}; }
  int value{};
};

struct A {
  int value{};
};

struct B : public A {};

struct C {
  int value{};
  explicit C(const A& a) : value(a.value) {}
};

struct D {
  int value{};
  explicit D(A&& a) : value(std::move(a.value)) {}
  D(const A& a) = delete;
};

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

  Wrapper<std::unique_ptr<int>> wrapper_of_unique_ptr(std::make_unique<int>(kConstInt));
  EXPECT_EQ(**wrapper_of_unique_ptr, kConstInt);

  Wrapper<std::mutex> wrapper_of_mutex(std::in_place);  // test it compiles
  std::ignore = wrapper_of_mutex;
}

TEST(TypedefTest, ImplicitConversionIsCorrect) {
  const int kValue = 1;

  {
    const Wrapper<B> wrapped_b(B{{kValue}});

    bool is_called = false;
    int value_called_on{};
    auto take_const_ref = [&is_called, &value_called_on](const Wrapper<A>& a) {
      is_called = true;
      value_called_on = a->value;
    };

    take_const_ref(wrapped_b);
    EXPECT_TRUE(is_called);
    EXPECT_EQ(value_called_on, kValue);
  }

  {
    Wrapper<B> wrapped_b(B{{kValue}});

    bool is_called = false;
    int value_called_on{};
    auto take_rvalue_ref = [&is_called, &value_called_on](Wrapper<A>&& a) {
      is_called = true;
      value_called_on = a->value;
    };

    take_rvalue_ref(std::move(wrapped_b));
    EXPECT_TRUE(is_called);
    EXPECT_EQ(value_called_on, kValue);
  }

  {
    Wrapper<A> wrapped_a(A{kValue});
    Wrapper<C> wrapped_c(wrapped_a);
    EXPECT_EQ(wrapped_c->value, kValue);
  }

  {
    Wrapper<A> wrapped_a(A{kValue});
    Wrapper<D> wrapped_c(std::move(wrapped_a));
    EXPECT_EQ(wrapped_c->value, kValue);
  }
}

TEST(TypedefTest, AssignmentIsCorrect) {
  const int kValue = 1;
  const int kValueOther = 1;
  {
    Wrapper<A> wrapped_a(A{kValue});
    Wrapper<A> wrapped_a_other(A{kValueOther});
    wrapped_a_other = wrapped_a;
    EXPECT_EQ(wrapped_a_other->value, kValueOther);
  }

  {
    Wrapper<A> wrapped_a(A{kValue});
    Wrapper<A> wrapped_a_other(A{kValueOther});
    wrapped_a_other = std::move(wrapped_a);
    EXPECT_EQ(wrapped_a_other->value, kValueOther);
  }

  {
    Wrapper<B> wrapped_b(B{{kValue}});
    Wrapper<A> wrapped_a_other(A{kValueOther});
    wrapped_a_other = wrapped_b;
    EXPECT_EQ(wrapped_a_other->value, kValueOther);
  }

  {
    Wrapper<B> wrapped_b(B{{kValue}});
    Wrapper<A> wrapped_a_other(A{kValueOther});
    wrapped_a_other = std::move(wrapped_b);
    EXPECT_EQ(wrapped_a_other->value, kValueOther);
  }
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
    auto add = [](int& i, int j) {
      int sum = i + j;
      i = j;
      return sum;
    };

    Wrapper<int> first(kFirst);
    Wrapper<int> second(kSecond);
    const Wrapper<int> sum_wrapped = Apply(add, first, second);
    EXPECT_EQ(*sum_wrapped, kSum);
    EXPECT_EQ(*first, kSecond);
    EXPECT_EQ(*second, kSecond);
  }

  {
    auto add = [](int&& i, int&& j) { return i + j; };

    Wrapper<int> first(kFirst);
    Wrapper<int> second(kSecond);
    const Wrapper<int> sum_wrapped = Apply(add, std::move(first), std::move(second));
    EXPECT_EQ(*sum_wrapped, kSum);
  }

  {
    auto add = [](const std::unique_ptr<int>& i, const std::unique_ptr<int>& j) { return *i + *j; };
    Wrapper<std::unique_ptr<int>> first(std::make_unique<int>(kFirst));
    Wrapper<std::unique_ptr<int>> second(std::make_unique<int>(kSecond));
    const Wrapper<int> sum_wrapped = Apply(add, first, second);
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
    int was_called_with = 0;
    auto returns_void = [&was_called, &was_called_with](int i) {
      was_called = true;
      was_called_with = i;
    };
    const Wrapper<void> void_wrapped = Apply(returns_void, kFirstWrapped);
    std::ignore = void_wrapped;
    EXPECT_TRUE(was_called);
    EXPECT_EQ(was_called_with, kFirst);
  }

  {
    Wrapper<Integer> first(Integer{kFirst});
    Wrapper<Integer> second(Integer{kSecond});
    Wrapper<Integer> sum_wrapped = Apply(&Integer::Add, first, second);
    EXPECT_EQ(sum_wrapped->value, kSum);
  }
}

}  // namespace orbit_base