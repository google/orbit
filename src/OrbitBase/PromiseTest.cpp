// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "OrbitBase/Promise.h"

namespace orbit_base {

TEST(Promise, IsValid) {
  Promise<void> promise{};
  EXPECT_TRUE(promise.IsValid());

  Promise<int> promise_int{};
  EXPECT_TRUE(promise_int.IsValid());
}

TEST(Promise, NewPromiseNotFinished) {
  Promise<void> promise{};
  EXPECT_FALSE(promise.IsFinished());
}

TEST(Promise, NewPromiseDoesNotHaveResult) {
  Promise<int> promise{};
  EXPECT_FALSE(promise.HasResult());
}

TEST(Promise, MarkFinished) {
  Promise<void> promise{};
  EXPECT_FALSE(promise.IsFinished());
  promise.MarkFinished();
  EXPECT_TRUE(promise.IsFinished());
}

TEST(Promise, SetResult) {
  Promise<int> promise{};
  EXPECT_FALSE(promise.HasResult());
  promise.SetResult(42);
  EXPECT_TRUE(promise.HasResult());
}

TEST(Promise, Move) {
  Promise<void> promise{};
  EXPECT_TRUE(promise.IsValid());

  auto promise2 = std::move(promise);
  EXPECT_FALSE(promise.IsValid());
  EXPECT_TRUE(promise2.IsValid());

  promise2.MarkFinished();

  auto promise3 = std::move(promise2);
  EXPECT_TRUE(promise3.IsFinished());
  EXPECT_FALSE(promise2.IsFinished());
}

TEST(Promise, MoveResult) {
  Promise<int> promise{};
  EXPECT_TRUE(promise.IsValid());

  auto promise2 = std::move(promise);
  EXPECT_FALSE(promise.IsValid());
  EXPECT_TRUE(promise2.IsValid());

  promise2.SetResult(42);

  auto promise3 = std::move(promise2);
  EXPECT_TRUE(promise3.HasResult());
  EXPECT_FALSE(promise2.HasResult());
}
}  // namespace orbit_base