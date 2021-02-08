// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <memory>

#include "MainThreadExecutorImpl.h"

namespace orbit_qt {

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

  bool called = false;
  auto future2 = executor->ScheduleAfter(future, [&called]() { called = true; });

  QCoreApplication::processEvents();
  EXPECT_FALSE(called);

  executor.reset();
  promise.MarkFinished();
  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
}

TEST(MainThreadExecutorImpl, Wait) {
  bool called = false;
  auto executor = MainThreadExecutorImpl::Create();
  orbit_base::Future<void> future = executor->Schedule([&called]() { called = true; });
  EXPECT_EQ(executor->WaitFor(future), MainThreadExecutor::WaitResult::kCompleted);
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

}  // namespace orbit_qt