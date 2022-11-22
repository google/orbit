// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"

namespace orbit_base {

TEST(ImmediateExecutor, ScheduledTaskShouldBeCalledImmediatelyWithVoid) {
  ImmediateExecutor executor{};
  bool called = false;
  auto future = executor.Schedule([&called]() { called = true; });
  EXPECT_TRUE(called);
  EXPECT_TRUE(future.IsFinished());
}

TEST(ImmediateExecutor, ScheduledTaskShouldBeCalledImmediatelyWithInt) {
  ImmediateExecutor executor{};
  bool called = false;
  auto future = executor.Schedule([&called]() {
    called = true;
    return 42;
  });
  EXPECT_TRUE(called);
  EXPECT_TRUE(future.IsFinished());
  EXPECT_EQ(future.Get(), 42);
}

TEST(ImmediateExecutor, ChainedTaskedShouldBeCalledImmediately) {
  ImmediateExecutor executor{};
  bool called = false;
  Promise<void> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfter(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());
  promise.MarkFinished();
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
}

TEST(ImmediateExecutor, ScheduleAfterIfSuccessShortCircuitOnErrorVoid) {
  ImmediateExecutor executor{};
  bool called = false;
  Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  EXPECT_FALSE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);
}

TEST(ImmediateExecutor, ScheduleAfterIfSuccessShortCircuitOnErrorInt) {
  ImmediateExecutor executor{};
  bool called = false;
  Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
    return 1 + value;
  });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  EXPECT_FALSE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);
}

TEST(ImmediateExecutor, ScheduleAfterIfSuccessCallOnSuccessVoid) {
  ImmediateExecutor executor{};
  bool called = false;
  Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(ImmediateExecutor, ScheduleAfterIfSuccessCallOnSuccessInt) {
  ImmediateExecutor executor{};
  bool called = false;
  Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
    return 1 + value;
  });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(42);
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().value(), 43);
}

}  // namespace orbit_base