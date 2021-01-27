// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"

namespace orbit_base {

TEST(Future, Create) {
  Promise<void> promise{};
  Future<void> future = promise.GetFuture();
  EXPECT_TRUE(future.IsValid());
}

TEST(Future, MarkFinished) {
  Promise<void> promise{};
  Future<void> future = promise.GetFuture();
  EXPECT_FALSE(future.IsFinished());

  promise.MarkFinished();
  EXPECT_TRUE(future.IsFinished());
}

TEST(Future, FinishedFutureResult) {
  Promise<int> promise{};
  Future<int> future = promise.GetFuture();
  EXPECT_FALSE(future.IsFinished());

  promise.SetResult(42);
  ASSERT_TRUE(future.IsFinished());
  EXPECT_EQ(future.Get(), 42);
}

TEST(Future, MoveBeforeResultSet) {
  Promise<int> promise{};
  Future<int> future = promise.GetFuture();
  EXPECT_FALSE(future.IsFinished());

  Future<int> future2 = std::move(future);
  EXPECT_FALSE(future.IsValid());
  EXPECT_FALSE(future.IsFinished());

  ASSERT_TRUE(future2.IsValid());
  EXPECT_FALSE(future2.IsFinished());

  promise.SetResult(42);
  ASSERT_TRUE(future2.IsValid());
  ASSERT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42);
}

TEST(Future, MoveAfterResultSet) {
  Promise<int> promise{};
  Future<int> future = promise.GetFuture();
  EXPECT_FALSE(future.IsFinished());

  promise.SetResult(42);
  ASSERT_TRUE(future.IsFinished());

  Future<int> future2 = std::move(future);
  EXPECT_FALSE(future.IsValid());
  EXPECT_FALSE(future.IsFinished());

  ASSERT_TRUE(future2.IsValid());
  ASSERT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42);
}

TEST(Future, RegisterContinuationOnInvalidFuture) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  auto future2 = std::move(future);
  EXPECT_TRUE(future2.IsValid());
  EXPECT_FALSE(future.IsValid());

  EXPECT_EQ(future.RegisterContinuation([]() {}),
            orbit_base::FutureRegisterContinuationResult::kFutureNotValid);
}

TEST(Future, RegisterContinuationOnValidButFinishedFuture) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  promise.MarkFinished();

  EXPECT_EQ(future.RegisterContinuation([]() {}),
            orbit_base::FutureRegisterContinuationResult::kFutureAlreadyCompleted);
}

TEST(Future, RegisterContinuationOnValidAndUnfinishedFuture) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  EXPECT_EQ(future.RegisterContinuation([]() {}),
            orbit_base::FutureRegisterContinuationResult::kSuccessfullyRegistered);
}

TEST(Future, CreateCompletedFuture) {
  orbit_base::Future<void> future{};
  EXPECT_TRUE(future.IsValid());
  EXPECT_TRUE(future.IsFinished());
}

TEST(Future, CreateCompletedFutureWithInt) {
  orbit_base::Future<int> future{42};
  EXPECT_TRUE(future.IsValid());
  EXPECT_TRUE(future.IsFinished());
  EXPECT_EQ(future.Get(), 42);
}
}  // namespace orbit_base