// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <optional>
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

  [[nodiscard]] static std::shared_ptr<MockExecutor> Create() {
    return std::make_shared<MockExecutor>();
  }
};

// Since testing::SaveArg can't deal with move-only arguments we define our own helper here.
template <typename T>
auto SaveUniquePtr(std::unique_ptr<T>* destination) {
  return [destination](std::unique_ptr<T> arg) { *destination = std::move(arg); };
}
}  // namespace

namespace orbit_base {

TEST(Executor, ScheduledTaskShouldBeCalledSimpleWithVoid) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  bool called = false;
  auto future = executor->Schedule([&called]() { called = true; });
  ASSERT_NE(action, nullptr);
  EXPECT_FALSE(called);
  EXPECT_FALSE(future.IsFinished());
  action->Execute();
  EXPECT_TRUE(called);
  EXPECT_TRUE(future.IsFinished());
}

TEST(Executor, ScheduledTaskShouldBeCalledSimpleWithInt) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  bool called = false;
  auto future = executor->Schedule([&called]() {
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
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<void> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfter(future, []() {});
  EXPECT_EQ(action, nullptr);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.MarkFinished();
  ASSERT_NE(action, nullptr);
  action->Execute();
  EXPECT_TRUE(chained_future.IsFinished());
}

TEST(Executor, ScheduleAfterIfSuccessShortCircuitOnErrorVoid) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, []() {});
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);

  EXPECT_EQ(executor->GetNumberOfWaitingContinuations(), 1);
  ASSERT_NE(action, nullptr);
  action->Execute();
  EXPECT_EQ(executor->GetNumberOfWaitingContinuations(), 0);
}

TEST(Executor, ScheduleAfterIfSuccessShortCircuitOnErrorInt) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [](int value) {
    EXPECT_EQ(value, 42);
    return 1 + value;
  });
  EXPECT_FALSE(chained_future.IsFinished());

  constexpr const char* const kErrorMessage{"Error"};
  promise.SetResult(ErrorMessage{kErrorMessage});
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_TRUE(chained_future.Get().has_error());
  EXPECT_EQ(chained_future.Get().error().message(), kErrorMessage);

  EXPECT_EQ(executor->GetNumberOfWaitingContinuations(), 1);
  ASSERT_NE(action, nullptr);
  action->Execute();
  EXPECT_EQ(executor->GetNumberOfWaitingContinuations(), 0);
}

TEST(Executor, ScheduleAfterIfSuccessCallOnSuccessVoid) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, []() {});
  EXPECT_EQ(action, nullptr);
  EXPECT_FALSE(chained_future.IsFinished());

  promise.SetResult(outcome::success());
  ASSERT_NE(action, nullptr);
  action->Execute();
  ASSERT_TRUE(chained_future.IsFinished());
  EXPECT_FALSE(chained_future.Get().has_error());
}

TEST(Executor, ScheduleAfterIfSuccessCallOnSuccessInt) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  Promise<ErrorMessageOr<int>> promise{};
  auto future = promise.GetFuture();
  auto chained_future = executor->ScheduleAfterIfSuccess(future, [](int value) {
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
  std::weak_ptr<Executor> executor{};

  bool called = false;
  std::optional<orbit_base::Future<void>> result =
      TrySchedule(executor, [&called]() { called = true; });

  EXPECT_FALSE(result.has_value());
  EXPECT_FALSE(called);
}

TEST(Executor, TrySchedule) {
  auto executor = MockExecutor::Create();
  std::unique_ptr<Action> action;
  EXPECT_CALL(*executor, ScheduleImpl(testing::_)).Times(1).WillOnce(SaveUniquePtr(&action));

  bool called = false;
  auto future_or_nullopt = TrySchedule(executor->weak_from_this(), [&called]() { called = true; });
  ASSERT_TRUE(future_or_nullopt.has_value());
  EXPECT_FALSE(future_or_nullopt->IsFinished());

  ASSERT_NE(action, nullptr);
  EXPECT_FALSE(called);
  action->Execute();
  EXPECT_TRUE(called);

  EXPECT_TRUE(future_or_nullopt->IsFinished());
}
}  // namespace orbit_base
