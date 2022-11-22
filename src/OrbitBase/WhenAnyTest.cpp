// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/WhenAny.h"

namespace orbit_base {

TEST(WhenAnyTest, OneFutureVoid) {
  Promise<void> promise0{};
  Future<void> future0 = promise0.GetFuture();

  Future<std::variant<std::monostate>> joined_future = WhenAny(future0);
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise0.MarkFinished();
  EXPECT_TRUE(joined_future.IsFinished());

  EXPECT_EQ(joined_future.Get().index(), 0);
}

TEST(WhenAnyTest, OneFuture) {
  Promise<int> promise0{};
  Future<int> future0 = promise0.GetFuture();

  Future<std::variant<int>> joined_future = WhenAny(future0);
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise0.SetResult(42);
  EXPECT_TRUE(joined_future.IsFinished());

  ASSERT_EQ(joined_future.Get().index(), 0);
  EXPECT_EQ(std::get<0>(joined_future.Get()), 42);
}

TEST(WhenAnyTest, ThreeFuturesFirstCompletes) {
  Promise<int> promise0{};
  Future<int> future0 = promise0.GetFuture();

  Promise<std::string> promise1{};
  Future<std::string> future1 = promise1.GetFuture();

  Promise<int> promise2{};
  Future<int> future2 = promise2.GetFuture();

  Future<std::variant<int, std::string, int>> joined_future = WhenAny(future0, future1, future2);
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise0.SetResult(42);
  EXPECT_TRUE(joined_future.IsFinished());
  ASSERT_EQ(joined_future.Get().index(), 0);
  EXPECT_EQ(std::get<0>(joined_future.Get()), 42);

  promise1.SetResult("Hello World");
  EXPECT_TRUE(joined_future.IsFinished());
  ASSERT_EQ(joined_future.Get().index(), 0);
  EXPECT_EQ(std::get<0>(joined_future.Get()), 42);

  promise2.SetResult(0);
  EXPECT_TRUE(joined_future.IsFinished());
  ASSERT_EQ(joined_future.Get().index(), 0);
  EXPECT_EQ(std::get<0>(joined_future.Get()), 42);
}

TEST(WhenAnyTest, ThreeFuturesSecondCompletes) {
  Promise<int> promise0{};
  Future<int> future0 = promise0.GetFuture();

  Promise<std::string> promise1{};
  Future<std::string> future1 = promise1.GetFuture();

  Promise<int> promise2{};
  Future<int> future2 = promise2.GetFuture();

  Future<std::variant<int, std::string, int>> joined_future = WhenAny(future0, future1, future2);
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise1.SetResult("Hello World");
  EXPECT_TRUE(joined_future.IsFinished());
  ASSERT_EQ(joined_future.Get().index(), 1);
  EXPECT_EQ(std::get<1>(joined_future.Get()), "Hello World");

  promise0.SetResult(42);
  EXPECT_TRUE(joined_future.IsFinished());
  ASSERT_EQ(joined_future.Get().index(), 1);
  EXPECT_EQ(std::get<1>(joined_future.Get()), "Hello World");

  promise2.SetResult(0);
  EXPECT_TRUE(joined_future.IsFinished());
  ASSERT_EQ(joined_future.Get().index(), 1);
  EXPECT_EQ(std::get<1>(joined_future.Get()), "Hello World");
}

TEST(WhenAnyTest, ThreeFuturesVoidCompletes) {
  Promise<int> promise0{};
  Future<int> future0 = promise0.GetFuture();

  Promise<std::string> promise1{};
  Future<std::string> future1 = promise1.GetFuture();

  Promise<void> promise2{};
  Future<void> future2 = promise2.GetFuture();

  Future<std::variant<int, std::string, std::monostate>> joined_future =
      WhenAny(future0, future1, future2);
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  promise2.MarkFinished();
  EXPECT_TRUE(joined_future.IsFinished());
  EXPECT_EQ(joined_future.Get().index(), 2);

  promise0.SetResult(42);
  EXPECT_TRUE(joined_future.IsFinished());
  EXPECT_EQ(joined_future.Get().index(), 2);

  promise1.SetResult("Hello World");
  EXPECT_TRUE(joined_future.IsFinished());
  EXPECT_EQ(joined_future.Get().index(), 2);
}
}  // namespace orbit_base
