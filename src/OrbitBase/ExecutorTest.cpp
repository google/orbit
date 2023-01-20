// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Action.h"
#include "OrbitBase/Executor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"

namespace {
class MockExecutor : public orbit_base::Executor {
 public:
  MOCK_METHOD(void, ScheduleImpl, (std::unique_ptr<Action> action), (override));

  [[nodiscard]] Handle GetExecutorHandle() const override { return handle_.Get(); }

 private:
  ScopedHandle handle_{this};
};

// Since testing::SaveArg can't deal with move-only arguments we define our own helper here.
template <typename T>
auto SaveUniquePtr(std::unique_ptr<T>* destination) {
  return [destination](std::unique_ptr<T> arg) { *destination = std::move(arg); };
}

// This is an error type - similar to ErrorMessage. We are not using ErrorMessage to make sure the
// logic works with some unknown generic error type.
struct ArbitraryError {
  std::string message;
};

template <typename T>
using ArbitraryErrorOr = Result<T, ArbitraryError>;
}  // namespace

namespace orbit_base {

TEST(Executor, ScheduledTaskShouldBeCalledSimpleWithVoid) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  bool called = false;
  auto future = executor.Schedule([&called]() { called = true; });
  ASSERT_NE(action, nullptr);
  EXPECT_FALSE(called);
  EXPECT_FALSE(future.IsFinished());
  action->Execute();
  EXPECT_TRUE(called);
  EXPECT_TRUE(future.IsFinished());
}

TEST(Executor, ScheduledTaskShouldBeCalledSimpleWithInt) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  bool called = false;
  auto future = executor.Schedule([&called]() {
    called = true;
    return 42;
  });

  ASSERT_NE(action, nullptr);
  EXPECT_FALSE(called);
  action->Execute();
  EXPECT_TRUE(called);
  ASSERT_TRUE(future.IsFinished());
  EXPECT_EQ(future.Get(), 42);
}

TEST(Executor, ChainedTaskedShouldBeCalledSimple) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<void> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfter(future, []() {});
  EXPECT_EQ(action, nullptr);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.MarkFinished();
  ASSERT_NE(action, nullptr);
  action->Execute();
  EXPECT_TRUE(chained_future.IsFinished());
}

TEST(Executor, ScheduleAfterIfSuccessShortCircuitOnErrorVoid) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ArbitraryErrorOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, []() {});
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ArbitraryError{kErrorMessage});
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message, kErrorMessage);

  EXPECT_EQ(executor.GetNumberOfWaitingContinuations(), 1);
  ASSERT_NE(action, nullptr);
  action->Execute();
  EXPECT_EQ(executor.GetNumberOfWaitingContinuations(), 0);
}

TEST(Executor, ScheduleAfterIfSuccessShortCircuitOnErrorInt) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ArbitraryErrorOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [](int value) {
    EXPECT_EQ(value, 42);
    return 1 + value;
  });
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ArbitraryError{kErrorMessage});
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message, kErrorMessage);

  EXPECT_EQ(executor.GetNumberOfWaitingContinuations(), 1);
  ASSERT_NE(action, nullptr);
  action->Execute();
  EXPECT_EQ(executor.GetNumberOfWaitingContinuations(), 0);
}

TEST(Executor, ScheduleAfterIfSuccessCallOnSuccessVoid) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ArbitraryErrorOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, []() {});
  EXPECT_EQ(action, nullptr);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  ASSERT_NE(action, nullptr);
  action->Execute();
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(Executor, ScheduleAfterIfSuccessCallOnSuccessInt) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ArbitraryErrorOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor.ScheduleAfterIfSuccess(future, [](int value) {
    EXPECT_EQ(value, 42);
    return 1 + value;
  });

  EXPECT_EQ(action, nullptr);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(42);
  ASSERT_NE(action, nullptr);
  EXPECT_FALSE(chained_future.IsFinished());

  action->Execute();
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().value(), 43);
}

TEST(Executor, TryScheduleFailing) {
  // The executor constructed by `MockExecutor::Create()` is a temporary, so it will go out of scope
  // after this line and the handle will become invalid - just what we want for this test.
  Executor::Handle handle = MockExecutor{}.GetExecutorHandle();

  bool called = false;
  std::optional<orbit_base::Future<void>> result =
      TrySchedule(handle, [&called]() { called = true; });

  EXPECT_FALSE(result.has_value());
  EXPECT_FALSE(called);
}

TEST(Executor, TrySchedule) {
  MockExecutor executor{};
  std::unique_ptr<Action> action;
  EXPECT_CALL(executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  bool called = false;
  auto future_or_nullopt =
      TrySchedule(executor.GetExecutorHandle(), [&called]() { called = true; });
  ASSERT_TRUE(future_or_nullopt.has_value());
  EXPECT_FALSE(future_or_nullopt->IsFinished());

  ASSERT_NE(action, nullptr);
  EXPECT_FALSE(called);
  action->Execute();
  EXPECT_TRUE(called);

  EXPECT_TRUE(future_or_nullopt->IsFinished());
}
}  // namespace orbit_base
