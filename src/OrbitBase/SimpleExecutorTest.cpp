// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SimpleExecutor.h"

namespace orbit_base {

TEST(SimpleExecutor, ScheduledTaskShouldBeCalledSimpleWithVoid) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  auto future = executor->Schedule([&called]() { called = true; });
  executor->ExecuteScheduledTasks();
  EXPECT_TRUE(called);
  EXPECT_TRUE(future.IsFinished());
}

TEST(SimpleExecutor, ScheduledTaskShouldBeCalledSimpleWithInt) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  auto future = executor->Schedule([&called]() {
    called = true;
    return 42;
  });
  executor->ExecuteScheduledTasks();
  EXPECT_TRUE(called);
  ASSERT_TRUE(future.IsFinished());
  EXPECT_EQ(future.Get(), 42);
}

TEST(SimpleExecutor, ChainedTaskedShouldBeCalledSimple) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  Promise<void> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfter(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());
  promise.MarkFinished();
  executor->ExecuteScheduledTasks();
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
}

TEST(SimpleExecutor, ScheduleAfterIfSuccessShortCircuitOnErrorVoid) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  EXPECT_TRUE(chained_future.IsFinished());

  executor->ExecuteScheduledTasks();
  EXPECT_FALSE(called);
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);
}

TEST(SimpleExecutor, ScheduleAfterIfSuccessShortCircuitOnErrorInt) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
    return 1 + value;
  });
  executor->ExecuteScheduledTasks();
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  executor->ExecuteScheduledTasks();
  EXPECT_FALSE(called);
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);
}

TEST(SimpleExecutor, ScheduleAfterIfSuccessCallOnSuccessVoid) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  executor->ExecuteScheduledTasks();
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  executor->ExecuteScheduledTasks();
  EXPECT_TRUE(called);
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(SimpleExecutor, ScheduleAfterIfSuccessCallOnSuccessInt) {
  auto executor = SimpleExecutor::Create();
  bool called = false;
  Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
    return 1 + value;
  });
  executor->ExecuteScheduledTasks();
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(42);
  executor->ExecuteScheduledTasks();
  EXPECT_TRUE(called);
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().value(), 43);
}

}  // namespace orbit_base
