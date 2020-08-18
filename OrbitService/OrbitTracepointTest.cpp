// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include <deque>
#include <vector>

#include "Utils.h"

using orbit_grpc_protos::TracepointInfo;

TEST(LinuxUtils, CategoriesTracepoints) {
  const auto tracepoint_infos = orbit_service::utils::ReadTracepoints();

  std::deque<std::string> categories;
  std::transform(tracepoint_infos.value().cbegin(), tracepoint_infos.value().cend(),
                 std::back_inserter(categories),
                 [](const TracepointInfo& value) { return value.category(); });

  ASSERT_TRUE(categories.size() > 0);

  static const std::array<std::string, 10> kCategoriesAvailable = {
      "sched",    "task",    "module",       "signal",     "sock",
      "syscalls", "migrate", "raw_syscalls", "exceptions", "iomap"};

  static const std::array<std::string, 3> kCategoriesUnavailable = {"orbit", "profiler",
                                                                    "instrumentation"};

  for (std::string category_available : kCategoriesAvailable) {
    ASSERT_TRUE(find(categories.begin(), categories.end(), category_available) != categories.end());
  }

  for (std::string category_unavailable : kCategoriesUnavailable) {
    ASSERT_TRUE(find(categories.begin(), categories.end(), category_unavailable) ==
                categories.end());
  }
}

TEST(LinuxUtils, NamesTracepoints) {
  const auto tracepoint_infos = orbit_service::utils::ReadTracepoints();

  std::deque<std::string> names;
  std::transform(tracepoint_infos.value().cbegin(), tracepoint_infos.value().cend(),
                 std::back_inserter(names),
                 [](const TracepointInfo& value) { return value.name(); });

  ASSERT_TRUE(names.size() > 0);

  static const std::array<std::string, 10> kNamesAvailable = {
      "sched_switch", "sched_wakeup",    "sched_process_fork", "sched_waking", "task_rename",
      "task_newtask", "signal_generate", "signal_deliver",     "timer_init",   "timer_start"};

  static const std::array<std::string, 3> kNamesUnavailable = {"orbit", "profiler",
                                                               "instrumentation"};

  for (std::string name_available : kNamesAvailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_available) != names.end());
  }

  for (std::string name_unavailable : kNamesUnavailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_unavailable) == names.end());
  }
}
