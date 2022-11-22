// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/SimpleExecutor.h"

#include <absl/synchronization/mutex.h>

#include <algorithm>
#include <memory>
#include <utility>

namespace orbit_base {
void SimpleExecutor::ScheduleImpl(std::unique_ptr<Action> action) {
  absl::MutexLock lock{&mutex_};
  scheduled_tasks_.emplace_back(std::move(action));
}

void SimpleExecutor::ExecuteScheduledTasks() {
  // Since each task can append to the list of scheduled tasks we have to make sure not to rely on
  // unstable iterators.
  absl::MutexLock lock{&mutex_};
  while (!scheduled_tasks_.empty()) {
    Action* action = scheduled_tasks_.front().get();
    {
      mutex_.Unlock();
      action->Execute();
      mutex_.Lock();
    }
    scheduled_tasks_.pop_front();
  }
}

std::shared_ptr<SimpleExecutor> SimpleExecutor::Create() {
  // NOLINTNEXTLINE
  return std::shared_ptr<SimpleExecutor>{new SimpleExecutor{}};
}
}  // namespace orbit_base
