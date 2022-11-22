// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"

namespace orbit_base {
TEST(RegisterContinuationOrCallDirectly, RegisteringSucceeds) {
  Promise<void> promise;
  auto future = promise.GetFuture();

  bool called = false;
  RegisterContinuationOrCallDirectly(future, [&called]() { called = true; });

  promise.MarkFinished();
  EXPECT_TRUE(called);
}

TEST(RegisterContinuationOrCallDirectly, DirectCallSucceeds) {
  Promise<void> promise;
  promise.MarkFinished();
  auto future = promise.GetFuture();

  bool called = false;
  RegisterContinuationOrCallDirectly(future, [&called]() { called = true; });
  EXPECT_TRUE(called);
}

TEST(UnwrapFuture, PassthroughWithVoid) {
  Promise<void> promise;
  auto future = promise.GetFuture();

  auto unwrapped_future = UnwrapFuture(future);
  EXPECT_EQ(future.IsFinished(), unwrapped_future.IsFinished());

  promise.MarkFinished();
  EXPECT_EQ(future.IsFinished(), unwrapped_future.IsFinished());
}

TEST(UnwrapFuture, PassthroughWithInt) {
  Promise<int> promise;
  auto future = promise.GetFuture();

  auto unwrapped_future = UnwrapFuture(future);
  EXPECT_EQ(future.IsFinished(), unwrapped_future.IsFinished());

  promise.SetResult(42);
  EXPECT_EQ(future.IsFinished(), unwrapped_future.IsFinished());
  EXPECT_EQ(future.Get(), unwrapped_future.Get());
}

TEST(UnwrapFuture, InnerFutureCompletesFirstWithVoid) {
  Promise<Future<void>> outer_promise;
  auto outer_future = outer_promise.GetFuture();

  auto unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<void> inner_promise;
  auto inner_future = inner_promise.GetFuture();

  outer_promise.SetResult(inner_future);
  EXPECT_FALSE(unwrapped_future.IsFinished());

  inner_promise.MarkFinished();
  EXPECT_TRUE(unwrapped_future.IsFinished());
}

TEST(UnwrapFuture, OuterFutureCompletesFirstWithVoid) {
  Promise<Future<void>> outer_promise;
  auto outer_future = outer_promise.GetFuture();

  auto unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<void> inner_promise;
  auto inner_future = inner_promise.GetFuture();

  inner_promise.MarkFinished();
  EXPECT_FALSE(unwrapped_future.IsFinished());

  outer_promise.SetResult(inner_future);
  EXPECT_TRUE(unwrapped_future.IsFinished());
}

TEST(UnwrapFuture, InnerFutureCompletesFirstWithInt) {
  Promise<Future<int>> outer_promise;
  auto outer_future = outer_promise.GetFuture();

  auto unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<int> inner_promise;
  auto inner_future = inner_promise.GetFuture();

  outer_promise.SetResult(inner_future);
  EXPECT_FALSE(unwrapped_future.IsFinished());

  inner_promise.SetResult(42);
  EXPECT_TRUE(unwrapped_future.IsFinished());
  EXPECT_EQ(unwrapped_future.Get(), 42);
}

TEST(UnwrapFuture, OuterFutureCompletesFirstWithInt) {
  Promise<Future<int>> outer_promise;
  auto outer_future = outer_promise.GetFuture();

  auto unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<int> inner_promise;
  auto inner_future = inner_promise.GetFuture();

  inner_promise.SetResult(42);
  EXPECT_FALSE(unwrapped_future.IsFinished());

  outer_promise.SetResult(inner_future);
  EXPECT_TRUE(unwrapped_future.IsFinished());
  EXPECT_EQ(unwrapped_future.Get(), 42);
}

// Future<ErrorMessageOr<Future<int>>> should unwrap to Future<ErrorMessageOr<int>>
TEST(UnwrapFuture, FutureOfErrorMessageOrFutureOfInt) {
  Promise<ErrorMessageOr<Future<int>>> outer_promise;
  Future<ErrorMessageOr<Future<int>>> outer_future = outer_promise.GetFuture();

  Future<ErrorMessageOr<int>> unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<int> inner_promise;
  Future<int> inner_future = inner_promise.GetFuture();

  inner_promise.SetResult(42);
  EXPECT_FALSE(unwrapped_future.IsFinished());

  outer_promise.SetResult(outcome::success(inner_future));
  EXPECT_TRUE(unwrapped_future.IsFinished());
  EXPECT_TRUE(unwrapped_future.Get().has_value());
  EXPECT_EQ(unwrapped_future.Get().value(), 42);
}

// Future<ErrorMessageOr<Future<ErrorMessageOr<int>>>> should unwrap to Future<ErrorMessageOr<int>>
TEST(UnwrapFuture, FutureOfErrorMessageOrFutureOfErrorMessageOrInt) {
  Promise<ErrorMessageOr<Future<ErrorMessageOr<int>>>> outer_promise;
  Future<ErrorMessageOr<Future<ErrorMessageOr<int>>>> outer_future = outer_promise.GetFuture();

  Future<ErrorMessageOr<int>> unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<ErrorMessageOr<int>> inner_promise;
  Future<ErrorMessageOr<int>> inner_future = inner_promise.GetFuture();

  inner_promise.SetResult(42);
  EXPECT_FALSE(unwrapped_future.IsFinished());

  outer_promise.SetResult(outcome::success(inner_future));
  EXPECT_TRUE(unwrapped_future.IsFinished());
  EXPECT_TRUE(unwrapped_future.Get().has_value());
  EXPECT_EQ(unwrapped_future.Get().value(), 42);
}

// Future<ErrorMessageOr<Future<ErrorMessageOr<int>>>> should unwrap to Future<ErrorMessageOr<int>>
TEST(UnwrapFuture, FutureOfErrorMessageOrFutureOfErrorMessageOrIntWithInnerError) {
  Promise<ErrorMessageOr<Future<ErrorMessageOr<int>>>> outer_promise;
  Future<ErrorMessageOr<Future<ErrorMessageOr<int>>>> outer_future = outer_promise.GetFuture();

  Future<ErrorMessageOr<int>> unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<ErrorMessageOr<int>> inner_promise;
  Future<ErrorMessageOr<int>> inner_future = inner_promise.GetFuture();

  inner_promise.SetResult(outcome::failure(ErrorMessage{"Error"}));
  EXPECT_FALSE(unwrapped_future.IsFinished());

  outer_promise.SetResult(outcome::success(inner_future));
  EXPECT_TRUE(unwrapped_future.IsFinished());
  EXPECT_TRUE(unwrapped_future.Get().has_error());
  EXPECT_EQ(unwrapped_future.Get().error().message(), "Error");
}

// Future<ErrorMessageOr<Future<ErrorMessageOr<int>>>> should unwrap to Future<ErrorMessageOr<int>>
TEST(UnwrapFuture, FutureOfErrorMessageOrFutureOfErrorMessageOrIntWithOuterError) {
  Promise<ErrorMessageOr<Future<ErrorMessageOr<int>>>> outer_promise;
  Future<ErrorMessageOr<Future<ErrorMessageOr<int>>>> outer_future = outer_promise.GetFuture();

  Future<ErrorMessageOr<int>> unwrapped_future = UnwrapFuture(outer_future);
  EXPECT_TRUE(unwrapped_future.IsValid());
  EXPECT_FALSE(unwrapped_future.IsFinished());

  Promise<ErrorMessageOr<int>> inner_promise;
  Future<ErrorMessageOr<int>> inner_future = inner_promise.GetFuture();

  outer_promise.SetResult(outcome::failure(ErrorMessage{"Error"}));
  EXPECT_TRUE(unwrapped_future.IsFinished());
  EXPECT_TRUE(unwrapped_future.Get().has_error());
  EXPECT_EQ(unwrapped_future.Get().error().message(), "Error");
}
}  // namespace orbit_base