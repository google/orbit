// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QMetaObject>
#include <QObject>
#include <Qt>
#include <vector>

#include "OrbitBase/Promise.h"
#include "QtUtils/WaitFor.h"

namespace orbit_qt_utils {

TEST(WaitFor, PendingFutureOfInt) {
  orbit_base::Promise<int> promise{};

  // We schedule a task on the event loop. This task will be executed when `WaitFor` processes
  // events while waiting.
  QObject context{};
  QMetaObject::invokeMethod(
      &context, [&]() { promise.SetResult(42); }, Qt::QueuedConnection);

  EXPECT_THAT(WaitFor(promise.GetFuture()), testing::Eq(42));
}

TEST(WaitFor, PendingFutureOfVoid) {
  orbit_base::Promise<void> promise{};

  // We schedule a task on the event loop. This task will be executed when `WaitFor` processes
  // events while waiting.
  QObject context{};
  QMetaObject::invokeMethod(
      &context, [&]() { promise.MarkFinished(); }, Qt::QueuedConnection);

  WaitFor(promise.GetFuture());
  SUCCEED();
}

TEST(WaitFor, CompletedFutureOfInt) {
  orbit_base::Promise<int> promise{};
  promise.SetResult(42);

  // We schedule a task on the event loop that we don't expect to be executed.
  QObject context{};
  QMetaObject::invokeMethod(
      &context, []() { FAIL(); }, Qt::QueuedConnection);

  // The returned future is already completed, so `WaitFor` will not process any scheduled tasks.
  EXPECT_THAT(WaitFor(promise.GetFuture()), testing::Eq(42));
}

TEST(WaitFor, CompletedFutureOfVoid) {
  orbit_base::Promise<void> promise{};
  promise.MarkFinished();

  // We schedule a task on the event loop that we don't expect to be executed.
  QObject context{};
  QMetaObject::invokeMethod(
      &context, []() { FAIL(); }, Qt::QueuedConnection);

  // The returned future is already completed, so `WaitFor` will not process any scheduled tasks.
  WaitFor(promise.GetFuture());
  SUCCEED();
}

}  // namespace orbit_qt_utils
