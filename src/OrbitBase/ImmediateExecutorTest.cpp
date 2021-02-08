// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/ImmediateExecutor.h"

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

}  // namespace orbit_base