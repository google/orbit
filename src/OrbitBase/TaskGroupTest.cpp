// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include <cstdint>
#include <vector>

#include "OrbitBase/TaskGroup.h"

TEST(TaskGroup, EmptyTaskGroup) {
  orbit_base::TaskGroup task_group;
  task_group.Wait();
}

TEST(TaskGroup, AllTasksAreCalledOnce) {
  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);

  orbit_base::TaskGroup task_group;
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
    orbit_base::TaskGroup task_group;
    for (uint32_t& counter : counters) {
      task_group.AddTask([&counter] { ++counter; });
    }
  }

  for (uint32_t counter : counters) {
    EXPECT_EQ(counter, 1);
  }
}
