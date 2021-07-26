// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_SIMPLE_EXECUTOR_H_
#define ORBIT_BASE_SIMPLE_EXECUTOR_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <deque>
#include <memory>

#include "OrbitBase/Action.h"
#include "OrbitBase/Executor.h"

namespace orbit_base {

// This is a simple implementation of Executor.
// It's mainly intended to be used in tests.
//
// The user can schedule tasks which get stored in SimpleExecutor
// until they call `ExecuteScheduledTasks` which will synchronously
// execute waiting tasks.
class SimpleExecutor : public Executor {
  explicit SimpleExecutor() = default;

  void ScheduleImpl(std::unique_ptr<Action> action) override;

 public:
  void ExecuteScheduledTasks();

  [[nodiscard]] static std::shared_ptr<SimpleExecutor> Create();

 private:
  absl::Mutex mutex_;
  std::deque<std::unique_ptr<Action>> scheduled_tasks_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_SIMPLE_EXECUTOR_H_