// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <deque>
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "ServiceUtils.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "services.pb.h"
#include "tracepoint.pb.h"

namespace orbit_service::utils {

using Path = std::filesystem::path;
using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(ServiceUtils, GetCumulativeTotalCpuTime) {
  // There is not much invariance here which we can test.
  // We know the optional should return a value and we know it's positive and
  // monotonically increasing.

  const auto& total_cpu_time1 = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(total_cpu_time1.has_value());
  ASSERT_TRUE(total_cpu_time1->jiffies.value > 0ul);
  ASSERT_TRUE(total_cpu_time1->cpus > 0ul);

  const auto& total_cpu_time2 = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(total_cpu_time2.has_value());
  ASSERT_TRUE(total_cpu_time2->jiffies.value > 0ul);
  ASSERT_TRUE(total_cpu_time2->cpus == total_cpu_time1->cpus);

  ASSERT_TRUE(total_cpu_time2->jiffies.value >= total_cpu_time1->jiffies.value);
}

TEST(ServiceUtils, GetCumulativeCpuTimeFromProcess) {
  const auto& jiffies1 = GetCumulativeCpuTimeFromProcess(getpid());
  ASSERT_TRUE(jiffies1.has_value());

  const auto& jiffies2 = GetCumulativeCpuTimeFromProcess(getpid());
  ASSERT_TRUE(jiffies2.has_value());

  ASSERT_TRUE(jiffies2->value >= jiffies1->value);

  const auto& total_cpu_time = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(total_cpu_time.has_value());
  ASSERT_TRUE(total_cpu_time->jiffies.value > 0ul);

  // A single process should never have consumed more CPU cycles than the total CPU time
  ASSERT_TRUE(jiffies2->value <= total_cpu_time->jiffies.value);
}

TEST(ServiceUtils, FindSymbolsFilePath) {
  const Path test_directory = orbit_test::GetTestdataDir();

  {
    // same file
    const Path module_path = test_directory / "hello_world_elf";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const auto result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), module_path);
  }

  {
    // separate file
    const Path module_path = test_directory / "no_symbols_elf";
    const Path symbols_path = test_directory / "no_symbols_elf.debug";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const auto result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), symbols_path);
  }

  {
    // non exising elf_file
    const Path module_path = test_directory / "not_existing_file";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const auto result = FindSymbolsFilePath(request);
    EXPECT_THAT(result, HasError("Unable to load ELF file"));
  }

  {
    // no build id, but does include symbols
    const Path module_path = test_directory / "hello_world_elf_no_build_id";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const auto result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), module_path);
  }

  {
    // no build id, no symbols
    const Path module_path = test_directory / "no_symbols_no_build_id";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const auto result = FindSymbolsFilePath(request);
    EXPECT_THAT(result, HasError("Module does not contain a build id"));
  }
}

TEST(ServiceUtils, CategoriesTracepoints) {
  using orbit_grpc_protos::TracepointInfo;

  if (getuid() != 0) {
    GTEST_SKIP() << "test is supported with root uid";
  }

  const auto tracepoint_infos = utils::ReadTracepoints();

  std::deque<std::string> categories;
  std::transform(tracepoint_infos.value().cbegin(), tracepoint_infos.value().cend(),
                 std::back_inserter(categories),
                 [](const TracepointInfo& value) { return value.category(); });

  ASSERT_GT(categories.size(), 0);
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

TEST(ServiceUtils, NamesTracepoints) {
  using orbit_grpc_protos::TracepointInfo;

  if (getuid() != 0) {
    GTEST_SKIP() << "test is supported with root uid";
  }

  const auto tracepoint_infos = utils::ReadTracepoints();

  std::deque<std::string> names;
  std::transform(tracepoint_infos.value().cbegin(), tracepoint_infos.value().cend(),
                 std::back_inserter(names),
                 [](const TracepointInfo& value) { return value.name(); });

  ASSERT_TRUE(names.size() > 0);
  static const std::array<std::string, 10> kNamesAvailable = {
      "sched_switch", "sched_wakeup",    "sched_process_fork", "sched_waking", "task_rename",
      "task_newtask", "signal_generate", "signal_deliver",     "timer_init",   "timer_start"};

  static const std::array<std::string, 5> kNamesUnavailable = {
      "orbit", "profiler", "instrumentation", "enable", "filter"};

  for (std::string name_available : kNamesAvailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_available) != names.end());
  }

  for (std::string name_unavailable : kNamesUnavailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_unavailable) == names.end());
  }
}

}  // namespace orbit_service::utils
