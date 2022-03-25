// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TASK_GROUP_H_
#define ORBIT_BASE_TASK_GROUP_H_

#include "OrbitBase/Executor.h"
#include "OrbitBase/ThreadPool.h"

namespace orbit_base {

// Simple class used to parallelize a group of tasks. Add tasks to be run in parallel then call
// "Wait()" to wait for all tasks to complete. Note that "Wait()" is called from the destructor if
// it wasn't explicitly called. This class is not thread-safe. A TaskGroup object should be owned
// and accessed by a single thread.
//
// Usage:
//
// void ProcessObjectsInParallel(std::vector<Object>& objects) {
//   TaskGroup task_group(executor_);
//   for (Object& object : objects) {
//     task_group.AddTask([&object]() { ProcessObject(object); });
//   }
// }
//
class TaskGroup {
 public:
  TaskGroup() : executor_(ThreadPool::GetDefaultThreadPool()) {}
  explicit TaskGroup(orbit_base::Executor* executor) : executor_(executor) {}
  ~TaskGroup() {
    if (!futures_.empty()) Wait();
  }

  TaskGroup(TaskGroup const&) = delete;
  TaskGroup& operator=(TaskGroup const&) = delete;

  template <typename T>
  Future<void> AddTask(T&& task) {
    return futures_.emplace_back(executor_->Schedule(std::forward<T>(task)));
  }

  void Wait() {
    for (Future<void>& future : futures_) {
      future.Wait();
    }
    futures_.clear();
  }

 private:
  orbit_base::Executor* executor_ = nullptr;
  std::vector<Future<void>> futures_;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_TASK_GROUP_H_
