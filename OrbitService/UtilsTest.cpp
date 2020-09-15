// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>

#include <deque>

#include "OrbitBase/Logging.h"
#include "Utils.h"
#include "absl/strings/str_format.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

namespace orbit_service::utils {

using Path = std::filesystem::path;

TEST(Utils, ReadModules) {
  const auto result = ReadModules(getpid());
  EXPECT_TRUE(result) << result.error().message();
}

TEST(Utils, ParseMaps) {
  using orbit_grpc_protos::ModuleInfo;

  {
    // Empty data
    const auto result = ParseMaps(std::string_view{""});
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_TRUE(result.value().empty());
  }

  const auto existing_elf_file_path = GetExecutablePath(getpid());
  ASSERT_TRUE(existing_elf_file_path) << existing_elf_file_path.error().message();
  const Path test_path = existing_elf_file_path.value().parent_path() / "testdata";
  const Path hello_world_path = test_path / "hello_world_elf";
  const Path text_file = test_path / "textfile.txt";

  {
    // Testing correct size of result. The last entry has a valid path, but the
    // executable flag is not set.
    const std::string data{absl::StrFormat(
        "7f687428f000-7f6874290000 r-xp 00009000 fe:01 661216                     "
        "/not/a/valid/file/path\n"
        "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     %s\n"
        "7f6874290001-7f6874297002 r-dp 00000000 fe:01 661214                     %s\n",
        hello_world_path, text_file)};
    const auto result = ParseMaps(data);
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value().size(), 1);
  }

  const Path no_symbols_path = test_path / "no_symbols_elf";
  {
    // Example data
    const std::string data{absl::StrFormat(
        "7f6874285000-7f6874288000 r--p 00000000 fe:01 661216                     %s\n"
        "7f6874288000-7f687428c000 r-xp 00003000 fe:01 661216                     %s\n"
        "7f687428c000-7f687428e000 r--p 00007000 fe:01 661216                     %s\n"
        "7f687428e000-7f687428f000 r--p 00008000 fe:01 661216                     %s\n"
        "7f687428f000-7f6874290000 rw-p 00009000 fe:01 661216                     %s\n"
        "0-1000 r-xp 00009000 fe:01 661216                     %s\n",
        hello_world_path, hello_world_path, hello_world_path, hello_world_path, hello_world_path,
        no_symbols_path)};

    const auto result = ParseMaps(data);
    ASSERT_TRUE(result) << result.error().message();
    ASSERT_EQ(result.value().size(), 2);

    const ModuleInfo* hello_module_info;
    const ModuleInfo* no_symbols_module_info;
    if (result.value()[0].name() == "hello_world_elf") {
      hello_module_info = &result.value()[0];
      no_symbols_module_info = &result.value()[1];
    } else {
      hello_module_info = &result.value()[1];
      no_symbols_module_info = &result.value()[0];
    }

    EXPECT_EQ(hello_module_info->name(), "hello_world_elf");
    EXPECT_EQ(hello_module_info->file_path(), hello_world_path);
    EXPECT_EQ(hello_module_info->file_size(), 16616);
    EXPECT_EQ(hello_module_info->address_start(), 0x7f6874285000);
    EXPECT_EQ(hello_module_info->address_end(), 0x7f6874290000);
    EXPECT_EQ(hello_module_info->build_id(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
    EXPECT_EQ(hello_module_info->load_bias(), 0x0);

    EXPECT_EQ(no_symbols_module_info->name(), "no_symbols_elf");
    EXPECT_EQ(no_symbols_module_info->file_path(), no_symbols_path);
    EXPECT_EQ(no_symbols_module_info->file_size(), 18768);
    EXPECT_EQ(no_symbols_module_info->address_start(), 0x0);
    EXPECT_EQ(no_symbols_module_info->address_end(), 0x1000);
    EXPECT_EQ(no_symbols_module_info->build_id(), "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    EXPECT_EQ(no_symbols_module_info->load_bias(), 0x400000);
  }
}

TEST(Utils, GetAllPids) {
  const auto pids = GetAllPids();

  // At least the test process needs to show up
  ASSERT_TRUE(pids.size() >= 1);

  {
    const auto result = std::find(pids.begin(), pids.end(), getpid());
    ASSERT_NE(result, pids.end());
  }

  // We also assume PID 1 is always present
  {
    const auto result = std::find(pids.begin(), pids.end(), 1);
    ASSERT_NE(result, pids.end());
  }
}

TEST(Utils, GetCumulativeTotalCpuTime) {
  // There is not much invariance here which we can test.
  // We know the optional should return a value and we know it's positive and
  // monotonically increasing.

  const auto& jiffies1 = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(jiffies1.has_value());
  ASSERT_TRUE(jiffies1->value > 0ul);

  const auto& jiffies2 = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(jiffies2.has_value());
  ASSERT_TRUE(jiffies2->value > 0ul);

  ASSERT_TRUE(jiffies2->value >= jiffies1->value);
}

TEST(Utils, GetCumulativeCpuTimeFromProcess) {
  const auto& jiffies1 = GetCumulativeCpuTimeFromProcess(getpid());
  ASSERT_TRUE(jiffies1.has_value());

  const auto& jiffies2 = GetCumulativeCpuTimeFromProcess(getpid());
  ASSERT_TRUE(jiffies2.has_value());

  ASSERT_TRUE(jiffies2->value >= jiffies1->value);

  const auto& jiffies_total = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(jiffies_total.has_value());
  ASSERT_TRUE(jiffies_total->value > 0ul);

  // A single process should never have consumed more CPU cycles than the total CPU time
  ASSERT_TRUE(jiffies2->value <= jiffies_total->value);
}

TEST(Utils, GetExecutablePath) {
  const auto result = GetExecutablePath(getpid());
  ASSERT_TRUE(result) << result.error().message();
  EXPECT_EQ(result.value().filename(), "OrbitServiceTests");
}

TEST(Utils, ReadFileToString) {
  {
    const auto result = ReadFileToString("non/existing/filename");
    ASSERT_FALSE(result);
  }

  {
    const auto executable_path = GetExecutablePath(getpid());
    ASSERT_TRUE(executable_path) << executable_path.error().message();
    const Path text_file = executable_path.value().parent_path() / "testdata/textfile.txt";
    const auto result = ReadFileToString(text_file);
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value(), "content\nnew line");
  }
}

TEST(Utils, FindSymbolsFilePath) {
  const auto executable_path = GetExecutablePath(getpid());
  ASSERT_TRUE(executable_path) << executable_path.error().message();
  const Path test_path = executable_path.value().parent_path() / "testdata";

  {
    // same file
    const Path hello_world_path = test_path / "hello_world_elf";
    const auto result = FindSymbolsFilePath(hello_world_path, {test_path});
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value(), hello_world_path);
  }

  {
    // separate file
    const Path no_symbols_path = test_path / "no_symbols_elf";
    const Path symbols_path = test_path / "no_symbols_elf.debug";
    const auto result = FindSymbolsFilePath(no_symbols_path, {test_path});
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value(), symbols_path);
  }

  {
    // non exising elf_file
    const Path not_existing_file = test_path / "not_existing_file";
    const auto result = FindSymbolsFilePath(not_existing_file, {test_path});
    ASSERT_FALSE(result);
    EXPECT_THAT(result.error().message(), testing::HasSubstr("Unable to load ELF file"));
  }

  {
    // no build id, but does include symbols
    const Path hello_world_elf_no_build_id = test_path / "hello_world_elf_no_build_id";
    const auto result = FindSymbolsFilePath(hello_world_elf_no_build_id, {test_path});
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value(), hello_world_elf_no_build_id);
  }

  {
    // no build id, no symbols
    const Path no_symbols_no_build_id = test_path / "no_symbols_no_build_id";
    const auto result = FindSymbolsFilePath(no_symbols_no_build_id, {test_path});
    ASSERT_FALSE(result);
    EXPECT_THAT(result.error().message(), testing::HasSubstr("Module does not contain a build id"));
  }
}

TEST(LinuxUtils, CategoriesTracepoints) {
  using orbit_grpc_protos::TracepointInfo;

  if (getuid() != 0) {
    GTEST_SKIP() << "test is supported with root uid";
  }

  const auto tracepoint_infos = utils::ReadTracepoints();

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

  static const std::array<std::string, 3> kNamesUnavailable = {"orbit", "profiler",
                                                               "instrumentation"};

  for (std::string name_available : kNamesAvailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_available) != names.end());
  }

  for (std::string name_unavailable : kNamesUnavailable) {
    ASSERT_TRUE(find(names.begin(), names.end(), name_unavailable) == names.end());
  }
}

}  // namespace orbit_service::utils
