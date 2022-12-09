// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <deque>
#include <iterator>
#include <string>
#include <vector>

#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Result.h"
#include "ReadTracepoints.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::TracepointInfo;

using orbit_test_utils::HasValue;

namespace orbit_tracepoint_service {

TEST(ServiceUtils, CategoriesTracepoints) {
  if (getuid() != 0) {
    GTEST_SKIP() << "test is supported with root uid";
  }

  const auto tracepoint_infos = ReadTracepoints();
  ASSERT_THAT(tracepoint_infos, HasValue());

  std::deque<std::string> categories;
  std::transform(tracepoint_infos.value().cbegin(), tracepoint_infos.value().cend(),
                 std::back_inserter(categories),
                 [](const TracepointInfo& value) { return value.category(); });

  ASSERT_FALSE(categories.empty());
  static const std::array<std::string, 10> categories_available = {
      "sched",    "task",    "module",       "signal",     "sock",
      "syscalls", "migrate", "raw_syscalls", "exceptions", "iomap"};

  static const std::array<std::string, 3> categories_unavailable = {"orbit", "profiler",
                                                                    "instrumentation"};

  for (const std::string& category_available : categories_available) {
    ASSERT_TRUE(find(categories.begin(), categories.end(), category_available) != categories.end());
  }

  for (const std::string& category_unavailable : categories_unavailable) {
    ASSERT_TRUE(find(categories.begin(), categories.end(), category_unavailable) ==
                categories.end());
  }
}

TEST(ServiceUtils, NamesTracepoints) {
  if (getuid() != 0) {
    GTEST_SKIP() << "test is supported with root uid";
  }

  const auto tracepoint_infos = ReadTracepoints();
  ASSERT_THAT(tracepoint_infos, HasValue());

  std::deque<std::string> names;
  std::transform(tracepoint_infos.value().cbegin(), tracepoint_infos.value().cend(),
                 std::back_inserter(names),
                 [](const TracepointInfo& value) { return value.name(); });

  ASSERT_FALSE(names.empty());
  static const std::array<std::string, 10> names_available = {
      "sched_switch", "sched_wakeup",    "sched_process_fork", "sched_waking", "task_rename",
      "task_newtask", "signal_generate", "signal_deliver",     "timer_init",   "timer_start"};

  static const std::array<std::string, 5> names_unavailable = {
      "orbit", "profiler", "instrumentation", "enable", "filter"};

  for (const std::string& name_available : names_available) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_available) != names.end());
  }

  for (const std::string& name_unavailable : names_unavailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_unavailable) == names.end());
  }
}

}  // namespace orbit_tracepoint_service