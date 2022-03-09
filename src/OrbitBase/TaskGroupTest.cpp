// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/TaskGroup.h"
#include "OrbitBase/UniqueResource.h"

using orbit_base::TaskGroup;
using orbit_base::ThreadPool;

namespace {

void ShutdownThreadPool(std::shared_ptr<ThreadPool> thread_pool) { thread_pool->ShutdownAndWait(); }

[[nodiscard]] ThreadPool* GetTestThreadPool() {
  constexpr size_t kNumThreads = 4;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  static orbit_base::unique_resource handle{
      ThreadPool::Create(kNumThreads, kNumThreads, kThreadTtl), ShutdownThreadPool};
  return handle.get().get();
}

}  // namespace

TEST(TaskGroup, EmptyTaskGroup) {
  TaskGroup task_group(GetTestThreadPool());
  task_group.Wait();
}

TEST(TaskGroup, AllTasksAreCalledOnce) {
  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);

  TaskGroup task_group(GetTestThreadPool());
  for (uint32_t& counter : counters) {
    task_group.AddTask([&counter] { ++counter; });
  }

  task_group.Wait();

  for (uint32_t counter : counters) {
    EXPECT_EQ(counter, 1);
  }
}

TEST(TaskGroup, AllTasksAreCalledOnceNoExplicitWait) {
  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);

  {
    TaskGroup task_group(GetTestThreadPool());
    for (uint32_t& counter : counters) {
      task_group.AddTask([&counter] { ++counter; });
    }
  }

  for (uint32_t counter : counters) {
    EXPECT_EQ(counter, 1);
  }
}
