// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/MainThreadExecutor.h"

namespace {

class ActionTestBase {
 public:
  ActionTestBase() = default;
  virtual ~ActionTestBase() = default;

  void set_value(int value) { value_ = value; }
  uint32_t value() const { return value_; }

  void IncrementValue() { value_++; }

 private:
  uint32_t value_;
};

class ActionTestDerived : public ActionTestBase {};

};  // namespace

TEST(MainThreadExecutor, Smoke) {
  std::unique_ptr<MainThreadExecutor> executor = MainThreadExecutor::Create();

  bool called = false;

  executor->Schedule([&]() { called = true; });

  EXPECT_FALSE(called);

  executor->ConsumeActions();

  EXPECT_TRUE(called);

  called = false;
  // There should be no actions to consume
  executor->ConsumeActions();

  EXPECT_FALSE(called);
}

TEST(MainThreadExecutor, MethodAction) {
  std::unique_ptr<MainThreadExecutor> executor = MainThreadExecutor::Create();

  ActionTestDerived action_test;
  action_test.set_value(42);

  executor->Schedule(&action_test, &ActionTestDerived::IncrementValue);

  EXPECT_EQ(action_test.value(), 42);
  executor->ConsumeActions();
  EXPECT_EQ(action_test.value(), 43);
}

TEST(MainThreadExecutor, CheckThread) {
  std::unique_ptr<MainThreadExecutor> executor = MainThreadExecutor::Create();

  // Let's make sure we execute action on main thread
  std::thread::id action_thread_id;

  std::thread scheduling_thread([&]() {
    executor->Schedule([&] { action_thread_id = std::this_thread::get_id(); });
  });

  scheduling_thread.join();

  EXPECT_NE(action_thread_id, std::this_thread::get_id());

  executor->ConsumeActions();

  EXPECT_EQ(action_thread_id, std::this_thread::get_id());
}

TEST(MainThreadExecutor, InvalidConsumer) {
  std::unique_ptr<MainThreadExecutor> executor;

  std::thread create_executor_thread(
      [&]() { executor = MainThreadExecutor::Create(); });

  create_executor_thread.join();

  EXPECT_DEATH(executor->ConsumeActions(), "");
}
