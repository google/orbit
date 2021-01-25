// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/Future.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Promise.h"
#include "absl/types/span.h"

namespace orbit_base {

TEST(JoinFutures, JoinEmptySpan) {
  Future<void> joined_future = JoinFutures({});
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_TRUE(joined_future.IsFinished());
}

TEST(JoinFutures, JoinSpanWithOneElement) {
  Promise<void> promise{};
  Future<void> future = promise.GetFuture();

  Future<void> joined_future = JoinFutures({future});
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise.MarkFinished();
  EXPECT_TRUE(joined_future.IsFinished());
}

TEST(JoinFutures, JoinSpanWithManyElements) {
  Promise<void> promise0{};
  Future<void> future0 = promise0.GetFuture();

  Promise<void> promise1{};
  Future<void> future1 = promise1.GetFuture();

  Promise<void> promise2{};
  Future<void> future2 = promise2.GetFuture();

  Future<void> joined_future = JoinFutures({future0, future1, future2});
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise0.MarkFinished();
  EXPECT_FALSE(joined_future.IsFinished());

  promise2.MarkFinished();
  EXPECT_FALSE(joined_future.IsFinished());

  promise1.MarkFinished();
  EXPECT_TRUE(joined_future.IsFinished());
}

TEST(JoinFutures, JoinSpanWithDuplicateElements) {
  Promise<void> promise{};
  Future<void> future = promise.GetFuture();

  Future<void> joined_future = JoinFutures({future, future});
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise.MarkFinished();
  EXPECT_TRUE(joined_future.IsFinished());
}

}  // namespace orbit_base