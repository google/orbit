// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "OrbitBase/Executor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "QtUtils/MainThreadExecutor.h"

namespace orbit_qt_utils {

TEST(MainThreadExecutor, Schedule) {
  MainThreadExecutor executor{};
  executor.Schedule([]() { QCoreApplication::exit(42); });

  EXPECT_EQ(QCoreApplication::exec(), 42);
}

TEST(MainThreadExecutor, ScheduleAfterAllVoid) {
  bool called = false;
  MainThreadExecutor executor{};
  auto future = executor.Schedule([&called]() { called = true; });
  executor.ScheduleAfter(future, []() { QCoreApplication::exit(42); });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(called);
}

TEST(MainThreadExecutor, ScheduleAfterCleansUpWaitingContinuations) {
  bool called = false;
  MainThreadExecutor executor{};
  auto future = executor.Schedule([&called]() { called = true; });
  executor.ScheduleAfter(future, []() { QCoreApplication::exit(42); });

  QCoreApplication::exec();
  EXPECT_EQ(executor.GetNumberOfWaitingContinuations(), 0);
}

TEST(MainThreadExecutor, ScheduleAfterWithIntegerBetweenJobs) {
  MainThreadExecutor executor{};
  auto future = executor.Schedule([]() { return 42; });
  executor.ScheduleAfter(future, [](int val) { QCoreApplication::exit(val); });

  EXPECT_EQ(QCoreApplication::exec(), 42);
}

TEST(MainThreadExecutor, ScheduleAfterWithIntegerAsFinalResultAndBetweenJobs) {
  MainThreadExecutor executor{};
  auto future = executor.Schedule([]() { return 42; });
  auto future2 = executor.ScheduleAfter(future, [](int val) {
    QCoreApplication::exit(val);
    return val + 42;
  });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42 + 42);
}

TEST(MainThreadExecutor, ScheduleAfterWithIntegerOnlyAsFinalResult) {
  bool called = false;
  MainThreadExecutor executor{};
  auto future = executor.Schedule([&called]() { called = true; });
  auto future2 = executor.ScheduleAfter(future, []() {
    QCoreApplication::exit(42);
    return 42 + 42;
  });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42 + 42);
  EXPECT_TRUE(called);
}

TEST(MainThreadExecutor, ScheduleAfterMultipleContinuations) {
  MainThreadExecutor executor{};
  auto future = executor.Schedule([]() { return 42; });
  auto future2 = executor.ScheduleAfter(future, [](int val) {
    EXPECT_EQ(val, 42);
    return val + 42;
  });
  auto future3 = executor.ScheduleAfter(future2, [](int val) {
    EXPECT_EQ(val, 2 * 42);
    return val + 42;
  });
  auto future4 = executor.ScheduleAfter(future3, [](int val) {
    EXPECT_EQ(val, 3 * 42);
    QCoreApplication::exit(val + 42);
  });

  EXPECT_EQ(QCoreApplication::exec(), 4 * 42);
}

TEST(MainThreadExecutor, ScheduleAfterWithExecutorOutOfScope) {
  orbit_base::Promise<void> promise;
  orbit_base::Future<void> future = promise.GetFuture();

  bool destructor_called = false;
  bool called = false;
  {
    MainThreadExecutor executor{};

    const auto deleter = [&destructor_called](const int* ptr) {
      destructor_called = true;
      delete ptr;  // NOLINT
    };
    std::unique_ptr<int, decltype(deleter)> unique_resource{new int{}, deleter};

    auto future2 = executor.ScheduleAfter(
        future, [&called, unique_resource = std::move(unique_resource)]() { called = true; });

    QCoreApplication::processEvents();
    EXPECT_FALSE(called);
    EXPECT_FALSE(destructor_called);
  }

  EXPECT_TRUE(destructor_called);
  promise.MarkFinished();
  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
}

TEST(MainThreadExecutor, ChainFuturesWithThen) {
  MainThreadExecutor executor{};
  const auto future = executor.Schedule([]() { return 42; });
  const auto future2 = future.Then(&executor, [](int val) {
    QCoreApplication::exit(val);
    return val + 42;
  });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42 + 42);
}

TEST(MainThreadExecutor, TrySchedule) {
  MainThreadExecutor executor{};
  const auto result =
      TrySchedule(executor.GetExecutorHandle(), []() { QCoreApplication::exit(42); });

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(QCoreApplication::exec(), 42);
}

TEST(MainThreadExecutor, TryScheduleFailing) {
  orbit_base::Executor::Handle handle = MainThreadExecutor{}.GetExecutorHandle();
  const auto result = TrySchedule(handle, []() {});
  EXPECT_FALSE(result.has_value());
}

TEST(MainThreadExecutor, ScheduleAfterIfSuccessShortCircuitOnErrorVoid) {
  MainThreadExecutor executor{};
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);
}

TEST(MainThreadExecutor, ScheduleAfterIfSuccessShortCircuitOnErrorInt) {
  MainThreadExecutor executor{};
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
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
  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);
}

TEST(MainThreadExecutor, ScheduleAfterIfSuccessCallOnSuccessVoid) {
  MainThreadExecutor executor{};
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  QCoreApplication::processEvents();
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(MainThreadExecutor, ScheduleAfterIfSuccessCallOnSuccessInt) {
  MainThreadExecutor executor{};
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [&called](int value) {
    EXPECT_EQ(value, 42);
    called = true;
    return 1 + value;
  });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(42);
  QCoreApplication::processEvents();
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().value(), 43);
}

TEST(MainThreadExecutor, ScheduleAfterIfSuccessTwice) {
  MainThreadExecutor executor{};

  bool first_called = false;
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto first_chained_future = executor.ScheduleAfterIfSuccess(future, [&first_called](int value) {
    EXPECT_EQ(value, 42);
    first_called = true;
    return std::to_string(value);
  });
  EXPECT_FALSE(first_called);
  EXPECT_FALSE(first_chained_future.IsFinished());

  bool second_called = false;
  auto second_chained_future = executor.ScheduleAfterIfSuccess(
      first_chained_future, [&first_called, &second_called](std::string_view number) {
        EXPECT_TRUE(first_called);
        EXPECT_EQ(number, "42");
        second_called = true;
        return std::string{"The number is "}.append(number);
      });
  EXPECT_FALSE(second_called);
  EXPECT_FALSE(second_chained_future.IsFinished());

  promise.SetResult(42);
  QCoreApplication::processEvents();
  QCoreApplication::processEvents();
  EXPECT_TRUE(second_chained_future.IsFinished());
  EXPECT_FALSE(second_chained_future.Get().has_error());
  EXPECT_EQ(second_chained_future.Get().value(), "The number is 42");
}

}  // namespace orbit_qt_utils