// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/MainThreadExecutor.h"

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
