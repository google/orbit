// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/MainThreadExecutor.h"

#include <list>
#include <thread>

#include "OrbitBase/Logging.h"
#include "absl/synchronization/mutex.h"

namespace {

class MainThreadExecutorImpl : public MainThreadExecutor {
 public:
  explicit MainThreadExecutorImpl(std::thread::id main_thread_id)
      : main_thread_id_(main_thread_id) {}

  void Schedule(std::unique_ptr<Action> action) override;

  void ConsumeActions() override;

 private:
  std::unique_ptr<Action> PopAction();

  std::thread::id main_thread_id_;
  absl::Mutex mutex_;
  std::list<std::unique_ptr<Action>> scheduled_actions_;
};

void MainThreadExecutorImpl::Schedule(std::unique_ptr<Action> action) {
  absl::MutexLock lock(&mutex_);
  scheduled_actions_.push_back(std::move(action));
}

std::unique_ptr<Action> MainThreadExecutorImpl::PopAction() {
  absl::MutexLock lock(&mutex_);
  if (scheduled_actions_.empty()) {
    return nullptr;
  }

  std::unique_ptr<Action> action = std::move(scheduled_actions_.front());
  scheduled_actions_.pop_front();

  return action;
}

void MainThreadExecutorImpl::ConsumeActions() {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  while (true) {
    std::unique_ptr<Action> action = PopAction();

    if (!action) {
      break;
    }

    action->Execute();
  }
}

};  // namespace

std::unique_ptr<MainThreadExecutor> MainThreadExecutor::Create(
    std::thread::id thread_id) {
  return std::make_unique<MainThreadExecutorImpl>(thread_id);
}
