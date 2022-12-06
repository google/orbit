// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

#include "OrbitBase/Action.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/ThreadPool.h"

using orbit_base::ThreadPool;

TEST(ThreadPool, Smoke) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<ThreadPool> thread_pool =
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
  std::shared_ptr<ThreadPool> thread_pool =
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
// TODO(https://github.com/google/orbit/issues/4503): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 5;
  constexpr size_t kThreadTtlMillis = 5;
  std::shared_ptr<ThreadPool> thread_pool = ThreadPool::Create(
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
// TODO(https://github.com/google/orbit/issues/4503): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 5;
  constexpr size_t kThreadTtlMillis = 5;
  std::shared_ptr<ThreadPool> thread_pool = ThreadPool::Create(
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

TEST(ThreadPool, CheckGetNumberOfBusyThreads) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr size_t kThreadTtlMillis = 5;

  std::shared_ptr<ThreadPool> thread_pool = ThreadPool::Create(
      kThreadPoolMinSize, kThreadPoolMaxSize, absl::Milliseconds(kThreadTtlMillis));

  EXPECT_EQ(thread_pool->GetNumberOfBusyThreads(), 0);

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
    // Schedule an action and check we have a busy thread
    absl::MutexLock lock(&actions_executed_mutex);
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 1; }, &actions_started),
        absl::Milliseconds(100)))
        << "actions_started=" << actions_started << ", expected 1";
    actions_started_mutex.Unlock();
    EXPECT_EQ(thread_pool->GetNumberOfBusyThreads(), 1);

    // Schedule another action and check that there are 2 workers threads now.
    thread_pool->Schedule(action);

    actions_started_mutex.Lock();
    EXPECT_TRUE(actions_started_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* started) { return *started == 2; }, &actions_started),
        absl::Milliseconds(100)));
    actions_started_mutex.Unlock();
    EXPECT_EQ(thread_pool->GetNumberOfBusyThreads(), 2);

    EXPECT_TRUE(actions_executed_mutex.AwaitWithTimeout(
        absl::Condition(
            +[](size_t* executed) { return *executed == 2; }, &actions_executed),
        absl::Milliseconds(100)))
        << "actions_executed=" << actions_executed << ", expected 2";
  }

  // Give it some time to finish the action
  absl::SleepFor(absl::Milliseconds(50));

  // Check there are no busy threads anymore
  EXPECT_EQ(thread_pool->GetNumberOfBusyThreads(), 0);

  thread_pool->ShutdownAndWait();
};

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

TEST(ThreadPool, NoShutdown) {
  auto thread_pool = ThreadPool::Create(1, 4, absl::Milliseconds(10));
}

TEST(ThreadPool, ScheduleAfterShutdown) {
  EXPECT_DEATH(
      {
        constexpr size_t kThreadPoolMinSize = 1;
        constexpr size_t kThreadPoolMaxSize = 2;
        constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
        std::shared_ptr<ThreadPool> thread_pool =
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
        std::shared_ptr<ThreadPool> thread_pool =
            ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

        thread_pool->Wait();
      },
      "");
}

TEST(ThreadPool, FutureBasic) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;
  bool called = false;

  {
    absl::MutexLock lock(&mutex);
    orbit_base::Future<void> future = thread_pool->Schedule([&]() {
      absl::MutexLock lock(&mutex);
      called = true;
    });

    EXPECT_TRUE(future.IsValid());
    EXPECT_FALSE(future.IsFinished());

    EXPECT_TRUE(mutex.AwaitWithTimeout(absl::Condition(
                                           +[](bool* called) { return *called; }, &called),
                                       absl::Milliseconds(100)));

    for (int elapsed_ms = 0; !future.IsFinished() && elapsed_ms < 100; ++elapsed_ms) {
      absl::SleepFor(absl::Milliseconds(1));
    }

    EXPECT_TRUE(future.IsValid());
    EXPECT_TRUE(future.IsFinished());
  }

  thread_pool->ShutdownAndWait();
}

TEST(ThreadPool, FutureContinuation) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;
  std::atomic_bool called = false;

  {
    absl::MutexLock lock(&mutex);
    orbit_base::Future<void> future =
        thread_pool->Schedule([&]() { absl::MutexLock lock(&mutex); });

    auto const result = future.RegisterContinuation([&]() { called.store(true); });
    EXPECT_EQ(result, orbit_base::FutureRegisterContinuationResult::kSuccessfullyRegistered);

    EXPECT_TRUE(future.IsValid());
    EXPECT_FALSE(future.IsFinished());

    EXPECT_TRUE(mutex.AwaitWithTimeout(
        absl::Condition(
            +[](std::atomic_bool* called) { return called->load(); }, &called),
        absl::Milliseconds(100)));

    future.Wait();
    EXPECT_TRUE(future.IsValid());
    EXPECT_TRUE(future.IsFinished());
  }

  thread_pool->ShutdownAndWait();
}

TEST(ThreadPool, FutureWithMoveOnlyResult) {
  class MoveOnlyInt {
    int value_;

   public:
    explicit MoveOnlyInt(int value) : value_(value) {}
    [[nodiscard]] int GetInt() const { return value_; }

    MoveOnlyInt(const MoveOnlyInt&) = delete;
    MoveOnlyInt& operator=(const MoveOnlyInt&) = delete;
    MoveOnlyInt(MoveOnlyInt&&) = default;
    MoveOnlyInt& operator=(MoveOnlyInt&&) = default;
    ~MoveOnlyInt() = default;
  };

  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;

  {
    mutex.Lock();
    orbit_base::Future<MoveOnlyInt> future = thread_pool->Schedule([&]() {
      absl::MutexLock lock(&mutex);
      return MoveOnlyInt{42};
    });

    EXPECT_TRUE(future.IsValid());
    EXPECT_FALSE(future.IsFinished());
    mutex.Unlock();

    EXPECT_TRUE(future.IsValid());
    EXPECT_EQ(future.Get().GetInt(), 42);
    EXPECT_TRUE(future.IsFinished());
  }

  thread_pool->ShutdownAndWait();
}

TEST(ThreadPool, WithRunActionParameter) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);

  std::atomic<int> run_before_action_count = 0;
  std::atomic<int> run_after_action_count = 0;
  auto run_action = [&run_before_action_count,
                     &run_after_action_count](const std::unique_ptr<Action>& action) {
    ++run_before_action_count;
    action->Execute();
    ++run_after_action_count;
  };

  std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl, run_action);

  absl::Mutex mutex;
  bool called = false;
  {
    absl::MutexLock called_lock(&mutex);
    int run_before_action_count_during_execution = -1;
    int run_after_action_count_during_execution = -1;

    thread_pool->Schedule([&]() {
      run_before_action_count_during_execution = run_before_action_count;
      run_after_action_count_during_execution = run_after_action_count;
      absl::MutexLock called_lock(&mutex);
      called = true;
    });

    EXPECT_FALSE(called);

    EXPECT_TRUE(mutex.AwaitWithTimeout(absl::Condition(&called), absl::Milliseconds(100)));

    EXPECT_EQ(run_before_action_count_during_execution, 1);
    EXPECT_EQ(run_after_action_count_during_execution, 0);
    EXPECT_TRUE(called);
    called = false;
  }

  thread_pool->ShutdownAndWait();

  EXPECT_FALSE(called);
  EXPECT_EQ(run_before_action_count, 1);
  EXPECT_EQ(run_after_action_count, 1);
}

TEST(ThreadPool, DefaultThreadPoolNotNull) {
  EXPECT_NE(ThreadPool::GetDefaultThreadPool(), nullptr);
}

TEST(ThreadPool, InitializeDefaultThreadPoolAfterFirstUse) {
  (void)ThreadPool::GetDefaultThreadPool();
  EXPECT_DEATH(ThreadPool::InitializeDefaultThreadPool(), "");
}

TEST(ThreadPool, SetDefaultThreadPoolAfterFirstUse) {
  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  static std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  (void)ThreadPool::GetDefaultThreadPool();
  EXPECT_DEATH(ThreadPool::SetDefaultThreadPool(thread_pool), "");
}

TEST(ThreadPool, SetNullDefaultThreadPool) {
  EXPECT_DEATH(ThreadPool::SetDefaultThreadPool(nullptr), "");
}
