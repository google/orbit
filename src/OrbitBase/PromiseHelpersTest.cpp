
// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>

#include "OrbitBase/Promise.h"
#include "OrbitBase/PromiseHelpers.h"

namespace orbit_base {

TEST(CallTaskAndSetResultInPromise, CallWithVoid) {
  Promise<void> promise;
  CallTaskAndSetResultInPromise<void> helper{&promise};

  bool called = false;
  helper.Call([&called]() { called = true; });

  EXPECT_TRUE(called);
}

TEST(CallTaskAndSetResultInPromise, CallWithInt) {
  Promise<int> promise;
  CallTaskAndSetResultInPromise<int> helper{&promise};

  bool called = false;
  helper.Call([&called]() {
    called = true;
    return 42;
  });

  EXPECT_TRUE(called);
}

TEST(CallTaskAndSetResultInPromise, CallWithMoveOnlyType) {
  Promise<std::unique_ptr<int>> promise;
  CallTaskAndSetResultInPromise<std::unique_ptr<int>> helper{&promise};

  bool called = false;
  helper.Call([&called]() {
    called = true;
    return std::make_unique<int>(42);
  });

  EXPECT_TRUE(called);
}

TEST(GetResultFromFutureAndCallContinuation, CallWithoutResult) {
  Promise<void> promise;
  promise.MarkFinished();
  auto future = promise.GetFuture();

  GetResultFromFutureAndCallContinuation<void> helper{&future};

  bool called = false;
  helper.Call([&called]() { called = true; });

  EXPECT_TRUE(called);
}

TEST(GetResultFromFutureAndCallContinuation, CallWithResult) {
  Promise<int> promise;
  promise.SetResult(42);
  auto future = promise.GetFuture();

  GetResultFromFutureAndCallContinuation<int> helper{&future};

  bool called = false;
  helper.Call([&called](int val) {
    EXPECT_EQ(val, 42);
    called = true;
  });

  EXPECT_TRUE(called);
}

}  // namespace orbit_base