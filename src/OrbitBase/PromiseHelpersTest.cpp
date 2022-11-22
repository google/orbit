
// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/PromiseHelpers.h"
#include "OrbitBase/Result.h"

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

TEST(HandleErrorAndSetResultInPromise, SuccessVoidInVoidOut) {
  ErrorMessageOr<void> in{outcome::success()};

  const auto invocable = []() -> void {};

  Promise<ErrorMessageOr<void>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<void>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  EXPECT_TRUE(future.Get().has_value());
}

TEST(HandleErrorAndSetResultInPromise, SuccessVoidInIntOut) {
  ErrorMessageOr<void> in{outcome::success()};

  const auto invocable = []() -> int { return 42; };

  Promise<ErrorMessageOr<int>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<int>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  ASSERT_TRUE(future.Get().has_value());
  EXPECT_EQ(future.Get().value(), 42);
}

TEST(HandleErrorAndSetResultInPromise, SuccessIntInVoidOut) {
  ErrorMessageOr<int> in{42};

  const auto invocable = [](int value) { EXPECT_EQ(value, 42); };

  Promise<ErrorMessageOr<void>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<void>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  EXPECT_TRUE(future.Get().has_value());
}

TEST(HandleErrorAndSetResultInPromise, SuccessIntInIntOut) {
  ErrorMessageOr<int> in{42};

  const auto invocable = [](int value) -> int {
    EXPECT_EQ(value, 42);
    return 42;
  };

  Promise<ErrorMessageOr<int>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<int>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  ASSERT_TRUE(future.Get().has_value());
  EXPECT_EQ(future.Get().value(), 42);
}

TEST(HandleErrorAndSetResultInPromise, FailureVoidInVoidOut) {
  ErrorMessageOr<void> in{ErrorMessage{"Error"}};

  const auto invocable = []() -> void { FAIL(); };

  Promise<ErrorMessageOr<void>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<void>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  ASSERT_TRUE(future.Get().has_error());
  EXPECT_EQ(future.Get().error().message(), "Error");
}

TEST(HandleErrorAndSetResultInPromise, FailureVoidInIntOut) {
  ErrorMessageOr<void> in{ErrorMessage{"Error"}};

  bool called = false;
  const auto invocable = [&called]() -> int {
    called = true;
    return 42;
  };

  Promise<ErrorMessageOr<int>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<int>> helper{&promise};
  helper.Call(invocable, in);
  EXPECT_FALSE(called);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  ASSERT_TRUE(future.Get().has_error());
  EXPECT_EQ(future.Get().error().message(), "Error");
}

TEST(HandleErrorAndSetResultInPromise, FailureIntInVoidOut) {
  ErrorMessageOr<int> in{ErrorMessage{"Error"}};

  const auto invocable = [](int value) {
    EXPECT_EQ(value, 42);
    FAIL();
  };

  Promise<ErrorMessageOr<void>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<void>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  ASSERT_TRUE(future.Get().has_error());
  EXPECT_EQ(future.Get().error().message(), "Error");
}

TEST(HandleErrorAndSetResultInPromise, FailureIntInIntOut) {
  ErrorMessageOr<int> in{ErrorMessage{"Error"}};

  const auto invocable = [](int value) -> int {
    EXPECT_EQ(value, 42);
    return 42;
  };

  Promise<ErrorMessageOr<int>> promise;
  HandleErrorAndSetResultInPromise<ErrorMessageOr<int>> helper{&promise};
  helper.Call(invocable, in);

  auto future = promise.GetFuture();
  ASSERT_TRUE(future.IsFinished());
  ASSERT_TRUE(future.Get().has_error());
  EXPECT_EQ(future.Get().error().message(), "Error");
}

}  // namespace orbit_base