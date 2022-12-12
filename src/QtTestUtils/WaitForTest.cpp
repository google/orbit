// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <chrono>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "QtTestUtils/WaitFor.h"

namespace orbit_qt_test_utils {

TEST(WaitFor, FinishesSuccessfully) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  // The lambda expression given to this invokeMethod call will be executed from the Qt event loop
  // the next it processes events. The earliest possible moment this can happen is on the WaitFor
  // call below. That way we know WaitFor processes Qt events while waiting.
  QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [promise = std::move(promise)]() mutable { promise.MarkFinished(); }, Qt::QueuedConnection);

  EXPECT_FALSE(future.IsFinished());
  const auto result = WaitFor(future);
  EXPECT_FALSE(HasTimedOut(result));
  EXPECT_TRUE(HasValue(result));
  EXPECT_THAT(result, YieldsNoTimeout());
}

TEST(WaitFor, FinishesSuccessfullyWithReturnValue) {
  orbit_base::Promise<int> promise{};
  orbit_base::Future<int> future = promise.GetFuture();

  // The lambda expression given to this invokeMethod call will be executed from the Qt event loop
  // the next it processes events. The earliest possible moment this can happen is on the WaitFor
  // call below. That way we know WaitFor processes Qt events while waiting.
  QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [promise = std::move(promise)]() mutable { promise.SetResult(42); }, Qt::QueuedConnection);

  EXPECT_FALSE(future.IsFinished());
  const auto result = WaitFor(future);
  EXPECT_FALSE(HasTimedOut(result));
  EXPECT_TRUE(HasValue(result));
  EXPECT_TRUE(GetValue(result).has_value());
  EXPECT_EQ(GetValue(result).value(), 42);
  EXPECT_THAT(result, YieldsResult(42));
}

TEST(WaitFor, TimesOut) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  EXPECT_FALSE(future.IsFinished());
  const auto result = WaitFor(future, std::chrono::milliseconds{5});
  EXPECT_TRUE(HasTimedOut(result));
  EXPECT_FALSE(HasValue(result));
  EXPECT_THAT(result, YieldsTimeout());
}

TEST(WaitFor, TimesOutWithReturnValue) {
  orbit_base::Promise<int> promise{};
  orbit_base::Future<int> future = promise.GetFuture();

  EXPECT_FALSE(future.IsFinished());
  const auto result = WaitFor(future, std::chrono::milliseconds{5});
  EXPECT_TRUE(HasTimedOut(result));
  EXPECT_FALSE(HasValue(result));
  EXPECT_THAT(result, YieldsTimeout());
}

}  // namespace orbit_qt_test_utils