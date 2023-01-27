// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <Qt>
#include <chrono>
#include <type_traits>
#include <utility>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "QtUtils/CreateTimeout.h"

using namespace std::chrono_literals;

namespace orbit_qt_test_utils {

TEST(WaitForWithTimeout, FinishesSuccessfully) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  // The lambda expression given to this invokeMethod call will be executed from the Qt event loop
  // the next it processes events. The earliest possible moment this can happen is on the WaitFor
  // call below. That way we know WaitFor processes Qt events while waiting.
  QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [promise = std::move(promise)]() mutable { promise.MarkFinished(); }, Qt::QueuedConnection);

  EXPECT_FALSE(future.IsFinished());
  const orbit_qt_utils::TimeoutOr<void> result = WaitForWithTimeout(future);
  EXPECT_THAT(result, YieldsNoTimeout());
}

TEST(WaitForWithTimeout, FinishesSuccessfullyWithReturnValue) {
  orbit_base::Promise<int> promise{};
  orbit_base::Future<int> future = promise.GetFuture();

  // The lambda expression given to this invokeMethod call will be executed from the Qt event loop
  // the next it processes events. The earliest possible moment this can happen is on the WaitFor
  // call below. That way we know WaitFor processes Qt events while waiting.
  QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [promise = std::move(promise)]() mutable { promise.SetResult(42); }, Qt::QueuedConnection);

  EXPECT_FALSE(future.IsFinished());
  orbit_qt_utils::TimeoutOr<int> result = WaitForWithTimeout(future);
  EXPECT_THAT(result, YieldsResult(42));
}

TEST(WaitForWithTimeout, TimesOut) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  EXPECT_FALSE(future.IsFinished());
  const orbit_qt_utils::TimeoutOr<void> result = WaitForWithTimeout(future, 5ms);
  EXPECT_THAT(result, YieldsTimeout());
}

TEST(WaitForWithTimeout, TimesOutWithReturnValue) {
  orbit_base::Promise<int> promise{};
  orbit_base::Future<int> future = promise.GetFuture();

  EXPECT_FALSE(future.IsFinished());
  const orbit_qt_utils::TimeoutOr<int> result = WaitForWithTimeout(future, 5ms);
  EXPECT_THAT(result, YieldsTimeout());
}

}  // namespace orbit_qt_test_utils