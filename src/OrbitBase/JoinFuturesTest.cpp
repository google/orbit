// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/Future.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Promise.h"
#include "absl/types/span.h"

namespace orbit_base {

template <typename T>
struct JoinFuturesTest : testing::Test {};

template <>
struct JoinFuturesTest<int> : testing::Test {
  using ValueType = int;
  using FutureValueType = std::vector<int>;

  static void FinishPromise(Promise<int>* promise, int index) { promise->SetResult(index); }
  static void VerifyResult(Future<std::vector<int>>* future, size_t size) {
    ASSERT_EQ(size, future->Get().size());
    for (size_t index = 0; index < future->Get().size(); ++index) {
      EXPECT_EQ(future->Get()[index], index);
    }
  }
};

template <>
struct JoinFuturesTest<void> : testing::Test {
  using ValueType = void;
  using FutureValueType = void;

  static void FinishPromise(Promise<void>* promise, int) { promise->MarkFinished(); }
  static void VerifyResult(Future<void>*, size_t) {
    // Nothing to verify when the result type is void
  }
};

using TestTypes = testing::Types<void, int>;
TYPED_TEST_SUITE(JoinFuturesTest, TestTypes);

TYPED_TEST(JoinFuturesTest, JoinEmptySpan) {
  using T = typename TestFixture::ValueType;        // Think int
  using R = typename TestFixture::FutureValueType;  // Think std::vector<int>

  Future<R> joined_future = JoinFutures(absl::Span<const Future<T>>{});
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_TRUE(joined_future.IsFinished());
}

TYPED_TEST(JoinFuturesTest, JoinSpanWithOneElement) {
  using T = typename TestFixture::ValueType;        // Think int
  using R = typename TestFixture::FutureValueType;  // Think std::vector<int>

  Promise<T> promise{};
  Future<T> future = promise.GetFuture();

  Future<R> joined_future = JoinFutures(absl::MakeConstSpan({future}));
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  TestFixture::FinishPromise(&promise, 0);
  EXPECT_TRUE(joined_future.IsFinished());
  TestFixture::VerifyResult(&joined_future, 1);
}

TYPED_TEST(JoinFuturesTest, JoinSpanWithManyElements) {
  using T = typename TestFixture::ValueType;        // Think int
  using R = typename TestFixture::FutureValueType;  // Think std::vector<int>

  Promise<T> promise0{};
  Future<T> future0 = promise0.GetFuture();

  Promise<T> promise1{};
  Future<T> future1 = promise1.GetFuture();

  Promise<T> promise2{};
  Future<T> future2 = promise2.GetFuture();

  Future<R> joined_future = JoinFutures(absl::MakeConstSpan({future0, future1, future2}));
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  TestFixture::FinishPromise(&promise0, 0);
  EXPECT_FALSE(joined_future.IsFinished());

  TestFixture::FinishPromise(&promise2, 2);
  EXPECT_FALSE(joined_future.IsFinished());

  TestFixture::FinishPromise(&promise1, 1);
  EXPECT_TRUE(joined_future.IsFinished());

  TestFixture::VerifyResult(&joined_future, 3);
}

TYPED_TEST(JoinFuturesTest, JoinSpanWithDuplicateElements) {
  using T = typename TestFixture::ValueType;        // Think int
  using R = typename TestFixture::FutureValueType;  // Think std::vector<int>

  Promise<T> promise{};
  Future<T> future = promise.GetFuture();

  Future<R> joined_future = JoinFutures(absl::MakeConstSpan({future, future}));
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_FALSE(joined_future.IsFinished());

  TestFixture::FinishPromise(&promise, 0);
  EXPECT_TRUE(joined_future.IsFinished());
}

TYPED_TEST(JoinFuturesTest, JoinSpanWithCompletedFutures) {
  using T = typename TestFixture::ValueType;        // Think int
  using R = typename TestFixture::FutureValueType;  // Think std::vector<int>

  Promise<T> promise0{};
  TestFixture::FinishPromise(&promise0, 0);
  Future<T> future0 = promise0.GetFuture();

  Promise<T> promise1{};
  TestFixture::FinishPromise(&promise1, 1);
  Future<T> future1 = promise1.GetFuture();

  Promise<T> promise2{};
  TestFixture::FinishPromise(&promise2, 2);
  Future<T> future2 = promise2.GetFuture();

  Future<R> joined_future = JoinFutures(absl::MakeConstSpan({future0, future1, future2}));
  EXPECT_TRUE(joined_future.IsValid());
  EXPECT_TRUE(joined_future.IsFinished());
  TestFixture::VerifyResult(&joined_future, 3);
}
}  // namespace orbit_base
