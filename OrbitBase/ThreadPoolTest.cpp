// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/ThreadPool.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"

TEST(ThreadPool, Smoke) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  std::unique_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize);

  absl::Mutex mutex;
  bool called = false;

  {
    absl::MutexLock lock(&mutex);
    thread_pool->Schedule([&]() {
      absl::MutexLock lock(&mutex);
      called = true;
    });

    EXPECT_FALSE(called);

    EXPECT_TRUE(mutex.AwaitWithTimeout(
        absl::Condition(
            +[](bool* called) { return *called; }, &called),
        absl::Milliseconds(100)));

    EXPECT_TRUE(called);
    called = false;
  }

  thread_pool->ShutdownAndWait();

  EXPECT_FALSE(called);
}

namespace {
class ActionTestBase {
 public:
  ActionTestBase() = default;
  virtual ~ActionTestBase() = default;

  void set_value(uint32_t value) {
    absl::MutexLock lock(&mutex_);
    value_ = value;
  }

  uint32_t value() {
    absl::MutexLock lock(&mutex_);
    return value_;
  }

  void IncrementValue() {
    absl::MutexLock lock(&mutex_);
    value_++;
  }

 private:
  absl::Mutex mutex_;
  uint32_t value_;
};

class ActionTestDerived : public ActionTestBase {};

};  // namespace

TEST(ThreadPool, MethodAction) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  std::unique_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize);

  ActionTestDerived action_test;
  action_test.set_value(40);

  thread_pool->Schedule(&action_test, &ActionTestDerived::IncrementValue);
  thread_pool->Schedule(&action_test, &ActionTestDerived::IncrementValue);

  thread_pool->ShutdownAndWait();

  EXPECT_EQ(action_test.value(), 42);
}

TEST(ThreadPool, QueuedActionsExecutedOnShutdown) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  std::unique_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize);

  absl::Mutex mutex;
  size_t counter = 0;

  constexpr size_t kNumberOfActions = 7;
  {
    absl::MutexLock lock(&mutex);
    for (size_t i = 0; i < kNumberOfActions; ++i) {
      thread_pool->Schedule([&]() {
        absl::MutexLock lock(&mutex);
        counter++;
      });
    }

    // All actions are waiting on the mutex, they won't resume until we unlock
    thread_pool->Shutdown();
  }

  thread_pool->Wait();

  EXPECT_EQ(counter, kNumberOfActions);
}

TEST(ThreadPool, ExtendThreadPool) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  std::unique_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize);

  absl::Mutex actions_started_mutex;
  size_t actions_started = 0;
  absl::Mutex actions_executed_mutex;
  size_t actions_executed = 0;

  auto action = [&] {
    actions_started_mutex.Lock();
    actions_started++;
    actions_started_mutex.Unlock();

    absl::MutexLock lock(&actions_executed_mutex);
    actions_executed++;
  };

  {
    // Schedule 2 actions and check we have one worker thread
    absl::MutexLock lock(&actions_executed_mutex);
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 1; }, &actions_started),
        absl::Milliseconds(100)))
        << "actions_started=" << actions_started << ", expected 1";
    actions_started_mutex.Unlock();

    // Schedule another action and check that there are 2 workers threads now.
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 2; }, &actions_started),
        absl::Milliseconds(100)));
    actions_started_mutex.Unlock();

    // Add more and make sure worker thread number remains kThreadPoolMaxSize
    for (size_t i = 0; i < 5; ++i) {
      thread_pool->Schedule(action);
    }

    // I do not think there is a way around it - sleep for 100ms
    absl::SleepFor(absl::Milliseconds(100));

    actions_started_mutex.Lock();
    EXPECT_EQ(actions_started, kThreadPoolMaxSize);
    actions_started_mutex.Unlock();
  }

  thread_pool->ShutdownAndWait();

  EXPECT_EQ(actions_started, 7);
  EXPECT_EQ(actions_executed, 7);
}

TEST(ThreadPool, InvalidArguments) {
  EXPECT_DEATH(ThreadPool::Create(0, 1), "");
  EXPECT_DEATH(ThreadPool::Create(2, 1), "");
  EXPECT_DEATH(ThreadPool::Create(0, 0), "");
}

TEST(ThreadPool, ScheduleAfterShutdown) {
  EXPECT_DEATH(
      {
        constexpr size_t kThreadPoolMinSize = 1;
        constexpr size_t kThreadPoolMaxSize = 2;
        std::unique_ptr<ThreadPool> thread_pool =
            ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize);

        thread_pool->Shutdown();
        thread_pool->Schedule([] {});
      },
      "");
}

TEST(ThreadPool, WaitWithoutShutdown) {
  EXPECT_DEATH(
      {
        constexpr size_t kThreadPoolMinSize = 1;
        constexpr size_t kThreadPoolMaxSize = 2;
        std::unique_ptr<ThreadPool> thread_pool =
            ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize);

        thread_pool->Wait();
      },
      "");
}
