// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/AnyErrorOf.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"

namespace orbit_base {
using orbit_test_utils::HasError;
using testing::Eq;

namespace {
// We are defining 6 arbitrary error types here. E1..E3 are copyable, while U1...U3 are move-only.
// All error types need to have `.message()` member function that returns something convertible to
// `std::string`.

template <typename T>
struct ErrorBase {
  [[nodiscard]] static std::string_view message() { return {}; }

  // The error types don't hold any state, so all instances are equal to each other.
  [[nodiscard]] friend bool operator==(const T& /*unused*/, const T& /*unused*/) { return true; }
  [[nodiscard]] friend bool operator!=(const T& /*unused*/, const T& /*unused*/) { return false; }
};

struct E1 : ErrorBase<E1> {};
struct E2 : ErrorBase<E2> {};
struct E3 : ErrorBase<E3> {};

template <typename T>
struct MoveOnlyErrorBase : ErrorBase<T> {
  MoveOnlyErrorBase() = default;
  MoveOnlyErrorBase(const MoveOnlyErrorBase&) = delete;
  MoveOnlyErrorBase(MoveOnlyErrorBase&&) = default;
  MoveOnlyErrorBase& operator=(const MoveOnlyErrorBase&) = delete;
  MoveOnlyErrorBase& operator=(MoveOnlyErrorBase&&) = default;
};

struct U1 : MoveOnlyErrorBase<U1> {};
struct U2 : MoveOnlyErrorBase<U2> {};
struct U3 : MoveOnlyErrorBase<U3> {};
}  // namespace

TEST(AnyErrorOf, CopyConstructionFromErrorType) {
  E1 error_value{};

  // Copy construction
  AnyErrorOf<E1, E2> error{error_value};

  EXPECT_TRUE(std::holds_alternative<E1>(error));
  EXPECT_EQ(error, E1{});
  EXPECT_NE(error, E2{});
}

TEST(AnyErrorOf, MoveConstructionFromErrorType) {
  // Move construction
  AnyErrorOf<U1, E2> error{U1{}};

  EXPECT_TRUE(std::holds_alternative<U1>(error));
  EXPECT_EQ(error, U1{});
  EXPECT_NE(error, E2{});
}

TEST(AnyErrorOf, CopyAssignmentFromErrorType) {
  E2 error_value{};
  AnyErrorOf<E1, E2> error{E1{}};

  // Copy assignment
  error = error_value;

  EXPECT_TRUE(std::holds_alternative<E2>(error));
  EXPECT_NE(error, E1{});
  EXPECT_EQ(error, E2{});
}

TEST(AnyErrorOf, MoveAssignmentFromErrorType) {
  AnyErrorOf<E1, U2> error{E1{}};

  // Move assignment
  error = U2{};

  EXPECT_TRUE(std::holds_alternative<U2>(error));
  EXPECT_NE(error, E1{});
  EXPECT_EQ(error, U2{});
}

TEST(AnyErrorOf, CopyConstructionFromCompatibleAnyErrorOf) {
  AnyErrorOf<E1, E2> source{E2{}};

  // Copy construction
  AnyErrorOf<E1, E2, E3> destination{source};

  EXPECT_TRUE(std::holds_alternative<E2>(destination));
  EXPECT_NE(destination, E1{});
  EXPECT_EQ(destination, E2{});
  EXPECT_NE(destination, E3{});
}

TEST(AnyErrorOf, MoveConstructionFromCompatibleAnyErrorOf) {
  AnyErrorOf<E1, U2> source{U2{}};

  // Move construction
  AnyErrorOf<E1, U2, E3> destination{std::move(source)};

  EXPECT_TRUE(std::holds_alternative<U2>(destination));
  EXPECT_NE(destination, E1{});
  EXPECT_EQ(destination, U2{});
  EXPECT_NE(destination, E3{});
}

TEST(AnyErrorOf, CopyAssignmentFromCompatibleAnyErrorOf) {
  AnyErrorOf<E1, E2> source{E2{}};
  AnyErrorOf<E1, E2, E3> destination{};

  // Copy assignment
  destination = source;

  EXPECT_TRUE(std::holds_alternative<E2>(destination));
  EXPECT_NE(destination, E1{});
  EXPECT_EQ(destination, E2{});
  EXPECT_NE(destination, E3{});
}

TEST(AnyErrorOf, MoveAssignmentFromCompatibleAnyErrorOf) {
  AnyErrorOf<U1, E2> source{E2{}};
  AnyErrorOf<U1, E2, E3> destination{};

  // Move assignment
  destination = std::move(source);

  EXPECT_TRUE(std::holds_alternative<E2>(destination));
  EXPECT_NE(destination, U1{});
  EXPECT_EQ(destination, E2{});
  EXPECT_NE(destination, E3{});
}

TEST(AnyErrorOf, OutcomeTryConstructsAnyErrorOfFromErrorType) {
  const auto converts_result = [&]() -> Result<void, AnyErrorOf<E1, U2>> {
    // Imagine we call a function that returns `ErrorMessageOr<void>`, but we have to return
    // `Result<void, AnyErrorOf<ErrorMessage, Canceled>>`. The needed conversion should be
    // seamless.
    OUTCOME_TRY((Result<void, E1>{E1{}}));
    return outcome::success();
  };

  EXPECT_THAT(converts_result(), HasError(Eq(E1{})));
}

TEST(AnyErrorOf, OutcomeTryConstructsAnyErrorOfFromCompatibleAnyErrorOf) {
  const auto converts_result = [&]() -> Result<void, AnyErrorOf<E1, E2, U3>> {
    // Imagine we call a function that returns `Result<void,AnyErrorOf<ErrorMessage, NotFound>>`,
    // but we have to return `Result<void, AnyErrorOf<ErrorMessage, Canceled, NotFound>>`.
    // The needed conversion should be seamless.
    OUTCOME_TRY((Result<void, AnyErrorOf<E1, E2>>{E1{}}));
    return outcome::success();
  };

  EXPECT_THAT(converts_result(), HasError(Eq(E1{})));
}
}  // namespace orbit_base
