// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "OrbitBase/Executor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "QtUtils/MainThreadExecutorImpl.h"

using orbit_base::MainThreadExecutor;

namespace orbit_qt_utils {

TEST(MainThreadExecutorImpl, Schedule) {
  auto executor = MainThreadExecutorImpl::Create();
  executor->Schedule([]() { QCoreApplication::exit(42); });

  EXPECT_EQ(QCoreApplication::exec(), 42);
}

TEST(MainThreadExecutorImpl, ScheduleAfterAllVoid) {
  bool called = false;
  auto executor = MainThreadExecutorImpl::Create();
  auto future = executor->Schedule([&called]() { called = true; });
  executor->ScheduleAfter(future, []() { QCoreApplication::exit(42); });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(called);
}

TEST(MainThreadExecutorImpl, ScheduleAfterCleansUpWaitingContinuations) {
  bool called = false;
  auto executor = MainThreadExecutorImpl::Create();
  auto future = executor->Schedule([&called]() { called = true; });
  executor->ScheduleAfter(future, []() { QCoreApplication::exit(42); });

  QCoreApplication::exec();
  EXPECT_EQ(executor->GetNumberOfWaitingContinuations(), 0);
}

TEST(MainThreadExecutorImpl, ScheduleAfterWithIntegerBetweenJobs) {
  auto executor = MainThreadExecutorImpl::Create();
  auto future = executor->Schedule([]() { return 42; });
  executor->ScheduleAfter(future, [](int val) { QCoreApplication::exit(val); });

  EXPECT_EQ(QCoreApplication::exec(), 42);
}

TEST(MainThreadExecutorImpl, ScheduleAfterWithIntegerAsFinalResultAndBetweenJobs) {
  auto executor = MainThreadExecutorImpl::Create();
  auto future = executor->Schedule([]() { return 42; });
  auto future2 = executor->ScheduleAfter(future, [](int val) {
    QCoreApplication::exit(val);
    return val + 42;
  });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42 + 42);
}

TEST(MainThreadExecutorImpl, ScheduleAfterWithIntegerOnlyAsFinalResult) {
  bool called = false;
  auto executor = MainThreadExecutorImpl::Create();
  auto future = executor->Schedule([&called]() { called = true; });
  auto future2 = executor->ScheduleAfter(future, []() {
    QCoreApplication::exit(42);
    return 42 + 42;
  });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42 + 42);
  EXPECT_TRUE(called);
}

TEST(MainThreadExecutorImpl, ScheduleAfterMultipleContinuations) {
  auto executor = MainThreadExecutorImpl::Create();
  auto future = executor->Schedule([]() { return 42; });
  auto future2 = executor->ScheduleAfter(future, [](int val) {
    EXPECT_EQ(val, 42);
    return val + 42;
  });
  auto future3 = executor->ScheduleAfter(future2, [](int val) {
    EXPECT_EQ(val, 2 * 42);
    return val + 42;
  });
  auto future4 = executor->ScheduleAfter(future3, [](int val) {
    EXPECT_EQ(val, 3 * 42);
    QCoreApplication::exit(val + 42);
  });

  EXPECT_EQ(QCoreApplication::exec(), 4 * 42);
}

TEST(MainThreadExecutorImpl, ScheduleAfterWithExecutorOutOfScope) {
  orbit_base::Promise<void> promise;
  orbit_base::Future<void> future = promise.GetFuture();

  auto executor = MainThreadExecutorImpl::Create();

  bool destructor_called = false;
  const auto deleter = [&destructor_called](const int* ptr) {
    destructor_called = true;
    delete ptr;  // NOLINT
  };
  std::unique_ptr<int, decltype(deleter)> unique_resource{new int{}, deleter};

  bool called = false;
  auto future2 = executor->ScheduleAfter(
      future, [&called, unique_resource = std::move(unique_resource)]() { called = true; });

  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
  EXPECT_FALSE(destructor_called);

  executor.reset();
  EXPECT_TRUE(destructor_called);
  promise.MarkFinished();
  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
}

TEST(MainThreadExecutorImpl, ChainFuturesWithThen) {
  auto executor = MainThreadExecutorImpl::Create();
  const auto future = executor->Schedule([]() { return 42; });
  const auto future2 = future.Then(executor.get(), [](int val) {
    QCoreApplication::exit(val);
    return val + 42;
  });

  EXPECT_EQ(QCoreApplication::exec(), 42);
  EXPECT_TRUE(future2.IsFinished());
  EXPECT_EQ(future2.Get(), 42 + 42);
}

TEST(MainThreadExecutorImpl, TrySchedule) {
  auto executor = MainThreadExecutorImpl::Create();
  const auto result = TrySchedule(executor, []() { QCoreApplication::exit(42); });

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(QCoreApplication::exec(), 42);
}

TEST(MainThreadExecutorImpl, TryScheduleWithInvalidWeakPtr) {
  const auto result = TrySchedule(std::weak_ptr<MainThreadExecutor>{}, []() {});
  EXPECT_FALSE(result.has_value());
}

TEST(MainThreadExecutorImpl, ScheduleAfterIfSuccessShortCircuitOnErrorVoid) {
  auto executor = MainThreadExecutorImpl::Create();
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called]() { called = true; });
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

TEST(MainThreadExecutorImpl, ScheduleAfterIfSuccessShortCircuitOnErrorInt) {
  auto executor = MainThreadExecutorImpl::Create();
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called](int value) {
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

TEST(MainThreadExecutorImpl, ScheduleAfterIfSuccessCallOnSuccessVoid) {
  auto executor = MainThreadExecutorImpl::Create();
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called]() { called = true; });
  EXPECT_FALSE(called);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  QCoreApplication::processEvents();
  EXPECT_TRUE(called);
  EXPECT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(MainThreadExecutorImpl, ScheduleAfterIfSuccessCallOnSuccessInt) {
  auto executor = MainThreadExecutorImpl::Create();
  bool called = false;
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [&called](int value) {
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

TEST(MainThreadExecutorImpl, ScheduleAfterIfSuccessTwice) {
  auto executor = MainThreadExecutorImpl::Create();

  bool first_called = false;
  orbit_base::Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto first_chained_future = executor->ScheduleAfterIfSuccess(future, [&first_called](int value) {
    EXPECT_EQ(value, 42);
    first_called = true;
    return std::to_string(value);
  });
  EXPECT_FALSE(first_called);
  EXPECT_FALSE(first_chained_future.IsFinished());

  bool second_called = false;
  auto second_chained_future = executor->ScheduleAfterIfSuccess(
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

TEST(MainThreadExecutorImpl, WaitForNoTimeout) {
  auto executor = MainThreadExecutorImpl::Create();
  orbit_base::Future<void> future = executor->Schedule([]() {});
  EXPECT_EQ(executor->WaitFor(future), MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForWithTimeout) {
  auto executor = MainThreadExecutorImpl::Create();
  orbit_base::Future<void> future = executor->Schedule([]() {});
  EXPECT_EQ(executor->WaitFor(future, std::chrono::milliseconds{10}),
            MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForNoTimeoutCompletedFutures) {
  auto executor = MainThreadExecutorImpl::Create();
  orbit_base::Future<void> future{};  // Constructs completed future
  EXPECT_EQ(executor->WaitFor(future), MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForWithTimeoutCompletedFutures) {
  auto executor = MainThreadExecutorImpl::Create();
  orbit_base::Future<void> future{};  // Constructs completed future
  EXPECT_EQ(executor->WaitFor(future, std::chrono::milliseconds{10}),
            MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForWithExceedingTimeout) {
  auto executor = MainThreadExecutorImpl::Create();
  // We will never mark that promise as finished, so we would wait forever if waiting didn't time
  // out.
  orbit_base::Promise<void> promise{};
  EXPECT_EQ(executor->WaitFor(promise.GetFuture(), std::chrono::milliseconds{10}),
            MainThreadExecutor::WaitResult::kTimedOut);
}

TEST(MainThreadExecutorImpl, WaitForWithAbort) {
  auto executor = MainThreadExecutorImpl::Create();
  // We will never mark that promise as finished, so we would wait forever if waiting wasn't
  // aborted.
  orbit_base::Promise<void> promise{};
  executor->Schedule([&]() { executor->AbortWaitingJobs(); });
  EXPECT_EQ(executor->WaitFor(promise.GetFuture()), MainThreadExecutor::WaitResult::kAborted);
}

TEST(MainThreadExecutorImpl, WaitForAllNoTimeout) {
  auto executor = MainThreadExecutorImpl::Create();
  std::array futures = {executor->Schedule([]() {}), executor->Schedule([]() {})};
  EXPECT_EQ(executor->WaitForAll(absl::MakeSpan(futures)),
            MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForAllWithTimeout) {
  auto executor = MainThreadExecutorImpl::Create();
  std::array futures = {executor->Schedule([]() {}), executor->Schedule([]() {})};
  EXPECT_EQ(executor->WaitForAll(absl::MakeSpan(futures), std::chrono::milliseconds{10}),
            MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForAllNoTimeoutCompletedFutures) {
  auto executor = MainThreadExecutorImpl::Create();
  std::array futures = {orbit_base::Future<void>{}, orbit_base::Future<void>{}};
  EXPECT_EQ(executor->WaitForAll(absl::MakeSpan(futures)),
            MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForAllWithTimeoutCompletedFutures) {
  auto executor = MainThreadExecutorImpl::Create();

  // Create completed futures
  std::array futures = {orbit_base::Future<void>{}, orbit_base::Future<void>{}};
  EXPECT_EQ(executor->WaitForAll(absl::MakeSpan(futures), std::chrono::milliseconds{10}),
            MainThreadExecutor::WaitResult::kCompleted);
}

TEST(MainThreadExecutorImpl, WaitForAllWithExceedingTimeout) {
  auto executor = MainThreadExecutorImpl::Create();

  // We won't mark these promises as finished, so we would wait forever if there wasn't a time out.
  std::array promises{orbit_base::Promise<void>{}, orbit_base::Promise<void>{}};
  std::array futures = {promises[0].GetFuture(), promises[1].GetFuture()};
  EXPECT_EQ(executor->WaitForAll(absl::MakeSpan(futures), std::chrono::milliseconds{10}),
            MainThreadExecutor::WaitResult::kTimedOut);
}

TEST(MainThreadExecutorImpl, WaitForAllWithAbort) {
  auto executor = MainThreadExecutorImpl::Create();

  // We won't mark these promises as finished, so we would wait forever if we didn't abort.
  std::array promises{orbit_base::Promise<void>{}, orbit_base::Promise<void>{}};
  std::array futures = {promises[0].GetFuture(), promises[1].GetFuture()};
  executor->Schedule([&]() { executor->AbortWaitingJobs(); });
  EXPECT_EQ(executor->WaitForAll(absl::MakeSpan(futures)),
            MainThreadExecutor::WaitResult::kAborted);
}

}  // namespace orbit_qt_utils