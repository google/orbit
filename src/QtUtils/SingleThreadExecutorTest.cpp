// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QThread>
#include <algorithm>
#include <chrono>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "QtUtils/SingleThreadExecutor.h"

namespace orbit_qt_utils {
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_qt_test_utils::YieldsTimeout;

TEST(SingleThreadExecutor, Schedule) {
  SingleThreadExecutor executor{};

  QThread* executing_thread{};
  orbit_base::Future<void> future =
      executor.Schedule([&]() { executing_thread = QThread::currentThread(); });

  future.Wait();
  EXPECT_EQ(executing_thread, executor.GetThread());
}

TEST(SingleThreadExecutor, ScheduleAfterOutOfLifetime) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future{};

  {
    SingleThreadExecutor executor{};
    // This schedules a task that is supposed to be executed after the promise completes
    // But the executor gets destroyed before the promise completes, so we expect the task to never
    // be executed.
    future = promise.GetFuture().Then(&executor, []() { FAIL(); });
  }

  promise.MarkFinished();

  // So this future never completes.
  EXPECT_THAT(WaitForWithTimeout(future, std::chrono::milliseconds{10}), YieldsTimeout());
}

}  // namespace orbit_qt_utils