// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"

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

TEST(Future, ThenWithVoid) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  future.Then(&executor, [&called]() { called = true; });

  EXPECT_FALSE(called);

  promise.MarkFinished();
  EXPECT_TRUE(called);
}

TEST(Future, ThenWithVoidFinished) {
  orbit_base::Future<void> future{};

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  future.Then(&executor, [&called]() { called = true; });

  EXPECT_TRUE(called);
}

TEST(Future, ThenWithInt) {
  orbit_base::Promise<int> promise{};
  orbit_base::Future<int> future = promise.GetFuture();

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  future.Then(&executor, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
  });

  EXPECT_FALSE(called);

  promise.SetResult(42);
  EXPECT_TRUE(called);
}

TEST(Future, ThenWithIntFinished) {
  orbit_base::Future<int> future{42};

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  future.Then(&executor, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
  });

  EXPECT_TRUE(called);
}

TEST(Future, ThenIfSuccessWithVoid) {
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  orbit_base::Future<ErrorMessageOr<void>> future = promise.GetFuture();

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  auto chained_future = future.ThenIfSuccess(&executor, [&called]() { called = true; });

  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
}

TEST(Future, ThenIfSuccessWithInt) {
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  orbit_base::Future<ErrorMessageOr<int>> future = promise.GetFuture();

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  auto chained_future = future.ThenIfSuccess(&executor, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
  });

  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(42);
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
}

TEST(Future, ThenIfSuccessWithVoidAndError) {
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  orbit_base::Future<ErrorMessageOr<void>> future = promise.GetFuture();

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  auto chained_future = future.ThenIfSuccess(&executor, [&called]() -> ErrorMessageOr<void> {
    called = true;
    return outcome::success();
  });

  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(Future, ThenIfSuccessWithIntAndError) {
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  orbit_base::Future<ErrorMessageOr<int>> future = promise.GetFuture();

  bool called = false;

  orbit_base::ImmediateExecutor executor{};
  auto chained_future =
      future.ThenIfSuccess(&executor, [&called](int value) -> ErrorMessageOr<int> {
        EXPECT_EQ(value, 42);
        called = true;
        return value;
      });

  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(42);
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(Future, FutureThenFutureWithError) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  static std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  bool called_b = false;
  bool called_c = false;

  auto future_a = thread_pool->Schedule([]() -> ErrorMessageOr<int> { return ErrorMessage{}; });

  auto future_b =
      future_a.ThenIfSuccess(thread_pool.get(), [&called_b](int value) -> ErrorMessageOr<int> {
        called_b = true;
        return value;
      });

  auto future_c = future_b.Then(thread_pool.get(),
                                [&called_c](ErrorMessageOr<int> result_b) -> ErrorMessageOr<int> {
                                  called_c = true;
                                  if (result_b.has_error()) {
                                    return ErrorMessage{};
                                  }
                                  return result_b.value();
                                });

  EXPECT_TRUE(future_c.Get().has_error());
  EXPECT_FALSE(called_b);
  EXPECT_TRUE(called_c);
}
}  // namespace orbit_base
