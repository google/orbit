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
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::unique_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;
  bool called = false;

  {
    absl::MutexLock lock(&mutex);
    thread_pool->Schedule([&]() {
      absl::MutexLock lock(&mutex);
      called = true;
    });

    EXPECT_FALSE(called);

    EXPECT_TRUE(mutex.AwaitWithTimeout(absl::Condition(
                                           +[](bool* called) { return *called; }, &called),
                                       absl::Milliseconds(100)));

    EXPECT_TRUE(called);
    called = false;
  }

  thread_pool->ShutdownAndWait();

  EXPECT_FALSE(called);
}

TEST(ThreadPool, QueuedActionsExecutedOnShutdown) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::unique_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

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

TEST(ThreadPool, CheckTtl) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 5;
  constexpr size_t kThreadTtlMillis = 5;
  std::unique_ptr<ThreadPool> thread_pool = ThreadPool::Create(
      kThreadPoolMinSize, kThreadPoolMaxSize, absl::Milliseconds(kThreadTtlMillis));

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

  constexpr size_t kNumberOfActions = 7;
  {
    absl::MutexLock lock(&actions_executed_mutex);
    for (size_t i = 0; i < kNumberOfActions; ++i) {
      thread_pool->Schedule(action);
    }
    // Wait until actions are on worker threads
    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 5; }, &actions_started),
        absl::Milliseconds(50)));
    actions_started_mutex.Unlock();

    // Now check the thread_pool size
    EXPECT_EQ(thread_pool->GetPoolSize(), kThreadPoolMaxSize);

    // Wait until all actions completed.
    EXPECT_TRUE(actions_executed_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* executed) { return *executed == 7; }, &actions_executed),
        absl::Milliseconds(50)));
  }

  // +10 because there might be some milliseconds between action is complete and
  // thread went idle
  absl::SleepFor(absl::Milliseconds(kThreadTtlMillis + 10));

  EXPECT_EQ(thread_pool->GetPoolSize(), kThreadPoolMinSize);

  thread_pool->ShutdownAndWait();
}

TEST(ThreadPool, ExtendThreadPool) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 5;
  constexpr size_t kThreadTtlMillis = 5;
  std::unique_ptr<ThreadPool> thread_pool = ThreadPool::Create(
      kThreadPoolMinSize, kThreadPoolMaxSize, absl::Milliseconds(kThreadTtlMillis));

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
    // Schedule an action and check we have one worker thread
    absl::MutexLock lock(&actions_executed_mutex);
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 1; }, &actions_started),
        absl::Milliseconds(100)))
        << "actions_started=" << actions_started << ", expected 1";
    actions_started_mutex.Unlock();

    EXPECT_EQ(thread_pool->GetPoolSize(), 1);

    // Schedule another action and check that there are 2 workers threads now.
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 2; }, &actions_started),
        absl::Milliseconds(100)));
    actions_started_mutex.Unlock();

    EXPECT_EQ(thread_pool->GetPoolSize(), 2);

    // Add more and make sure worker thread number remains kThreadPoolMaxSize
    for (size_t i = 0; i < 10; ++i) {
      thread_pool->Schedule(action);
    }

    // I do not think there is a way around it - sleep for 50ms
    absl::SleepFor(absl::Milliseconds(50));

    EXPECT_EQ(thread_pool->GetPoolSize(), kThreadPoolMaxSize);

    actions_started_mutex.Lock();
    EXPECT_EQ(actions_started, kThreadPoolMaxSize);
    actions_started_mutex.Unlock();

    EXPECT_TRUE(actions_executed_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* executed) { return *executed == 12; }, &actions_executed),
        absl::Milliseconds(100)))
        << "actions_executed=" << actions_executed << ", expected 12";
  }

  // Wait for ttl+10 and check that ThreadPool has reduced
  // number of worker threads
  absl::SleepFor(absl::Milliseconds(kThreadTtlMillis + 10));
  EXPECT_EQ(thread_pool->GetPoolSize(), kThreadPoolMinSize);

  EXPECT_EQ(actions_started, 12);
  EXPECT_EQ(actions_executed, 12);

  // Now make sure thread_pool increases number of threads
  // correctly after reducing number of worker threads.
  {
    absl::MutexLock lock(&actions_executed_mutex);
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 13; }, &actions_started),
        absl::Milliseconds(100)));
    actions_started_mutex.Unlock();

    EXPECT_EQ(thread_pool->GetPoolSize(), 1);

    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 14; }, &actions_started),
        absl::Milliseconds(100)));
    actions_started_mutex.Unlock();

    EXPECT_EQ(thread_pool->GetPoolSize(), 2);

    // Add more and make sure worker thread number remains kThreadPoolMaxSize
    for (size_t i = 0; i < 10; ++i) {
      thread_pool->Schedule(action);
    }

    // I do not think there is a way around it - sleep for 50ms
    absl::SleepFor(absl::Milliseconds(50));

    EXPECT_EQ(thread_pool->GetPoolSize(), kThreadPoolMaxSize);
    actions_started_mutex.Lock();
    EXPECT_EQ(actions_started, 12 + kThreadPoolMaxSize);
    actions_started_mutex.Unlock();
  }

  thread_pool->ShutdownAndWait();

  EXPECT_EQ(actions_started, 24);
  EXPECT_EQ(actions_executed, 24);
}

TEST(ThreadPool, InvalidArguments) {
  EXPECT_DEATH(
      {
        auto thread_pool = ThreadPool::Create(0, 1, absl::Milliseconds(1));
        thread_pool->ShutdownAndWait();
      },
      "");
  EXPECT_DEATH(
      {
        auto thread_pool = ThreadPool::Create(2, 1, absl::Milliseconds(1));
        thread_pool->ShutdownAndWait();
      },
      "");
  EXPECT_DEATH(
      {
        auto thread_pool = ThreadPool::Create(0, 0, absl::Milliseconds(1));
        thread_pool->ShutdownAndWait();
      },
      "");
  EXPECT_DEATH(
      {
        auto thread_pool = ThreadPool::Create(1, 2, absl::Milliseconds(0));
        thread_pool->ShutdownAndWait();
      },
      "");
  EXPECT_DEATH(
      {
        auto thread_pool = ThreadPool::Create(1, 2, absl::Nanoseconds(999));
        thread_pool->ShutdownAndWait();
      },
      "");
}

TEST(ThreadPool, NoShutdown) { EXPECT_DEATH(ThreadPool::Create(1, 4, absl::Milliseconds(10)), ""); }

TEST(ThreadPool, ScheduleAfterShutdown) {
  EXPECT_DEATH(
      {
        constexpr size_t kThreadPoolMinSize = 1;
        constexpr size_t kThreadPoolMaxSize = 2;
        constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
        std::unique_ptr<ThreadPool> thread_pool =
            ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

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
        constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
        std::unique_ptr<ThreadPool> thread_pool =
            ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

        thread_pool->Wait();
      },
      "");
}
