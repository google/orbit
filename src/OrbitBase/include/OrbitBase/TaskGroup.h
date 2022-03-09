// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TASK_GROUP_H_
#define ORBIT_BASE_TASK_GROUP_H_

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
//   TaskGroup task_group(thread_pool_);
//   for (Object& object : objects) {
//     task_group.AddTask([&object]() { ProcessObject(object); });
//   }
// }
//
class TaskGroup {
 public:
  explicit TaskGroup(orbit_base::ThreadPool* thread_pool) : thread_pool_(thread_pool) {}
  ~TaskGroup() {
    if (!done_) Wait();
  }

  TaskGroup() = delete;
  TaskGroup(TaskGroup const&) = delete;
  TaskGroup& operator=(TaskGroup const&) = delete;

  template <typename T>
  Future<void>* AddTask(T&& task) {
    return &futures_.emplace_back(thread_pool_->Schedule(std::forward<T>(task)));
  }

  void Wait() {
    for (Future<void>& future : futures_) {
      future.Wait();
    }
    done_ = true;
  }

 private:
  orbit_base::ThreadPool* thread_pool_ = nullptr;
  std::vector<Future<void>> futures_;
  bool done_ = false;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_TASK_GROUP_H_
