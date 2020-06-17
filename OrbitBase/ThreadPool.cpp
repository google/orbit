// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/ThreadPool.h"

#include <list>
#include <thread>

#include "OrbitBase/Logging.h"
#include "absl/synchronization/mutex.h"

namespace {

class ThreadPoolImpl : public ThreadPool {
 public:
  explicit ThreadPoolImpl(size_t thread_pool_min_size,
                          size_t thread_pool_max_size);

  void Schedule(std::unique_ptr<Action> action) override;
  void Shutdown() override;
  void Wait() override;

 private:
  bool ActionsAvailableOrShutdownInitiated();
  // Blocking call - returns nullptr if shutdown initiated.
  // This also decrements idle_threads_ counter (in should be
  // done in the same scope as poping action from the queue).
  std::unique_ptr<Action> TakeAction();
  void CreateWorker();
  void WorkerFunction();

  absl::Mutex mutex_;
  std::list<std::unique_ptr<Action>> scheduled_actions_;
  std::vector<std::thread> worker_threads_;
  size_t thread_pool_max_size_;
  size_t idle_threads_;
  bool shutdown_initiated_;
};

ThreadPoolImpl::ThreadPoolImpl(size_t thread_pool_min_size,
                               size_t thread_pool_max_size)
    : thread_pool_max_size_(thread_pool_max_size),
      idle_threads_(0),
      shutdown_initiated_(false) {
  CHECK(thread_pool_min_size > 0);
  CHECK(thread_pool_max_size >= thread_pool_min_size);

  absl::MutexLock lock(&mutex_);
  for (size_t i = 0; i < thread_pool_min_size; ++i) {
    CreateWorker();
  }
}

void ThreadPoolImpl::CreateWorker() {
  CHECK(!shutdown_initiated_);
  idle_threads_++;
  worker_threads_.emplace_back([this] { WorkerFunction(); });
}

void ThreadPoolImpl::Schedule(std::unique_ptr<Action> action) {
  absl::MutexLock lock(&mutex_);
  CHECK(!shutdown_initiated_);

  scheduled_actions_.push_back(std::move(action));
  if (idle_threads_ < scheduled_actions_.size() &&
      worker_threads_.size() < thread_pool_max_size_) {
    CreateWorker();
  }
}

void ThreadPoolImpl::Shutdown() {
  absl::MutexLock lock(&mutex_);
  shutdown_initiated_ = true;
}

void ThreadPoolImpl::Wait() {
  absl::MutexLock lock(&mutex_);
  CHECK(shutdown_initiated_);

  while (!worker_threads_.empty()) {
    std::thread thread = std::move(worker_threads_.back());
    worker_threads_.pop_back();
    mutex_.Unlock();
    thread.join();
    mutex_.Lock();
  }
}

bool ThreadPoolImpl::ActionsAvailableOrShutdownInitiated() {
  return !scheduled_actions_.empty() || shutdown_initiated_;
}

std::unique_ptr<Action> ThreadPoolImpl::TakeAction() {
  absl::MutexLock lock(&mutex_);
  mutex_.Await(absl::Condition(
      this, &ThreadPoolImpl::ActionsAvailableOrShutdownInitiated));

  if (scheduled_actions_.empty()) {
    return nullptr;
  }

  std::unique_ptr<Action> action = std::move(scheduled_actions_.front());
  scheduled_actions_.pop_front();

  CHECK(idle_threads_ > 0);  // Sanity check
  // Note: do not move this outside of this method. This has to be done in
  // the same lock scope as taking an action from the queue to avoid a situation
  // where the check in Schedule() does not create a worker thread because if it
  // is called between taking action from a queue and reducing idle_thread_
  // counter the idle_threads_ < scheduled_actions_.size() check will
  // be false and the action will end up being stuck in the queue for some
  // time.
  --idle_threads_;

  return action;
}

void ThreadPoolImpl::WorkerFunction() {
  while (true) {
    std::unique_ptr<Action> action = TakeAction();

    if (!action) {
      break;
    }

    action->Execute();

    absl::MutexLock lock(&mutex_);
    ++idle_threads_;
  }
}

};  // namespace

std::unique_ptr<ThreadPool> ThreadPool::Create(size_t thread_pool_min_size,
                                               size_t thread_pool_max_size) {
  return std::make_unique<ThreadPoolImpl>(thread_pool_min_size,
                                          thread_pool_max_size);
}
