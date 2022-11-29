// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <QObject>
#include <QTimer>
#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/ThreadPool.h"
#include "QtUtils/FutureWatcher.h"

namespace orbit_qt_utils {

TEST(FutureWatcher, WaitFor) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  QTimer completion_timer{};
  QObject::connect(&completion_timer, &QTimer::timeout, [&]() { promise.MarkFinished(); });
  completion_timer.start(std::chrono::milliseconds{0});

  FutureWatcher watcher{};
  EXPECT_EQ(watcher.WaitFor(std::move(future), std::chrono::milliseconds{40}),
            FutureWatcher::Reason::kFutureCompleted);
}

TEST(FutureWatcher, WaitForWithTimeout) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  QTimer completion_timer{};
  QObject::connect(&completion_timer, &QTimer::timeout, [&]() { promise.MarkFinished(); });
  completion_timer.start(std::chrono::milliseconds{80});

  FutureWatcher watcher{};
  EXPECT_EQ(watcher.WaitFor(std::move(future), std::chrono::milliseconds{40}),
            FutureWatcher::Reason::kTimeout);
}

TEST(FutureWatcher, WaitForWithAbort) {
  orbit_base::Promise<void> promise{};
  orbit_base::Future<void> future = promise.GetFuture();

  QTimer completion_timer{};
  QObject::connect(&completion_timer, &QTimer::timeout, [&]() { promise.MarkFinished(); });
  completion_timer.start(std::chrono::milliseconds{80});

  FutureWatcher watcher{};

  QTimer abort_timer{};
  QObject::connect(&abort_timer, &QTimer::timeout, &watcher, &FutureWatcher::Abort);
  abort_timer.start(std::chrono::milliseconds{10});

  EXPECT_EQ(watcher.WaitFor(std::move(future), std::chrono::milliseconds{40}),
            FutureWatcher::Reason::kAbortRequested);
}

TEST(FutureWatcher, WaitForWithThreadPool) ABSL_NO_THREAD_SAFETY_ANALYSIS {
  // One background job which is supposed to succeed. (No timeout, no abort)

  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<orbit_base::ThreadPool> thread_pool =
      orbit_base::ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;

  mutex.Lock();
  orbit_base::Future<void> future = thread_pool->Schedule([&]() { absl::MutexLock lock(&mutex); });

  EXPECT_TRUE(future.IsValid());
  EXPECT_FALSE(future.IsFinished());

  // The lambda will be executed by the event loop, running inside of watcher.WaitFor.
  QTimer::singleShot(std::chrono::milliseconds{5},
                     [&]() ABSL_NO_THREAD_SAFETY_ANALYSIS { mutex.Unlock(); });

  FutureWatcher watcher{};
  const auto reason = watcher.WaitFor(std::move(future), std::nullopt);

  EXPECT_EQ(reason, FutureWatcher::Reason::kFutureCompleted);

  thread_pool->ShutdownAndWait();
}

TEST(FutureWatcher, WaitForWithThreadPoolAndTimeout) {
  // One background job which is supposed to time out.

  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<orbit_base::ThreadPool> thread_pool =
      orbit_base::ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;

  mutex.Lock();
  orbit_base::Future<void> future = thread_pool->Schedule([&]() { absl::MutexLock lock(&mutex); });

  EXPECT_TRUE(future.IsValid());
  EXPECT_FALSE(future.IsFinished());

  FutureWatcher watcher{};
  const auto reason = watcher.WaitFor(future, std::chrono::milliseconds{5});

  EXPECT_EQ(reason, FutureWatcher::Reason::kTimeout);

  mutex.Unlock();
  thread_pool->ShutdownAndWait();
}

TEST(FutureWatcher, WaitForAllWithThreadPool) ABSL_NO_THREAD_SAFETY_ANALYSIS {
  // Multiple background jobs which are all supposed to succeed.

  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<orbit_base::ThreadPool> thread_pool =
      orbit_base::ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;
  mutex.Lock();

  std::vector<orbit_base::Future<void>> futures;
  for (int i = 0; i < 10; ++i) {
    futures.emplace_back(thread_pool->Schedule([&]() { absl::MutexLock lock(&mutex); }));

    orbit_base::Future<void>& future = futures.back();
    EXPECT_TRUE(future.IsValid());
    EXPECT_FALSE(future.IsFinished());
  }

  // The lambda will be executed by the event loop, running inside of watcher.WaitFor.
  QTimer::singleShot(std::chrono::milliseconds{5},
                     [&]() ABSL_NO_THREAD_SAFETY_ANALYSIS { mutex.Unlock(); });

  FutureWatcher watcher{};
  const auto reason = watcher.WaitForAll(absl::MakeSpan(futures), std::nullopt);

  EXPECT_EQ(reason, FutureWatcher::Reason::kFutureCompleted);

  thread_pool->ShutdownAndWait();
}

TEST(FutureWatcher, WaitForAllWithThreadPoolAndTimeout) ABSL_NO_THREAD_SAFETY_ANALYSIS {
  // Multiple background jobs which are all supposed to time out.

  constexpr size_t kThreadPoolMinSize = 1;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<orbit_base::ThreadPool> thread_pool =
      orbit_base::ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  absl::Mutex mutex;
  mutex.Lock();

  std::vector<orbit_base::Future<void>> futures;
  for (int i = 0; i < 10; ++i) {
    futures.emplace_back(thread_pool->Schedule([&]() {
      absl::MutexLock lock(&mutex);
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }));

    orbit_base::Future<void>& future = futures.back();
    EXPECT_TRUE(future.IsValid());
    EXPECT_FALSE(future.IsFinished());
  }

  // The lambda will be executed by the event loop, running inside of watcher.WaitFor.
  QTimer::singleShot(std::chrono::milliseconds{5},
                     [&]() ABSL_NO_THREAD_SAFETY_ANALYSIS { mutex.Unlock(); });

  // The timer starts execution of background jobs after 5ms. Every background job takes 1ms and
  // they can't run in parallel due to the mutex. That means in total 15ms of execution time is
  // needed, but the watcher will time out after 10ms.

  FutureWatcher watcher{};
  const auto reason = watcher.WaitForAll(absl::MakeSpan(futures), std::chrono::milliseconds{10});

  EXPECT_EQ(reason, FutureWatcher::Reason::kTimeout);

  // This function will wait until all jobs are finished. That means the whole test will need a
  // little longer than 15ms.
  thread_pool->ShutdownAndWait();
}

TEST(FutureWatcher, WaitForAllWithEmptyList) {
  FutureWatcher watcher{};
  const auto reason = watcher.WaitForAll({}, std::nullopt);

  EXPECT_EQ(reason, FutureWatcher::Reason::kFutureCompleted);
}

}  // namespace orbit_qt_utils
