// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/ascii.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include "OrbitClientData/ProcessData.h"
#include "module.pb.h"
#include "process.pb.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

TEST(ProcessData, Constructor) {
  int32_t pid = 10;
  const std::string name = "Process name";
  const double cpu_usage = 55.5;
  const std::string full_path = "/example/path";
  const std::string command_line = "/example/path --argument";
  const bool is_64_bit = true;

  ProcessInfo process_info;
  process_info.set_pid(pid);
  process_info.set_name(name);
  process_info.set_cpu_usage(cpu_usage);
  process_info.set_full_path(full_path);
  process_info.set_command_line(command_line);
  process_info.set_is_64_bit(is_64_bit);

  ProcessData process(process_info);

  EXPECT_EQ(process.pid(), pid);
  EXPECT_EQ(process.name(), name);
  EXPECT_EQ(process.cpu_usage(), cpu_usage);
  EXPECT_EQ(process.full_path(), full_path);
  EXPECT_EQ(process.command_line(), command_line);
  EXPECT_EQ(process.is_64_bit(), is_64_bit);
}

TEST(ProcessData, DefaultConstructor) {
  ProcessData process;

  EXPECT_EQ(process.pid(), -1);
  EXPECT_EQ(process.name(), "");
  EXPECT_EQ(process.cpu_usage(), 0);
  EXPECT_EQ(process.full_path(), "");
  EXPECT_EQ(process.command_line(), "");
  EXPECT_EQ(process.is_64_bit(), false);
}

TEST(ProcessData, UpdateModuleInfos) {
  {
    // valid module infos
    const std::string file_path_1 = "filepath1";
    uint64_t start_address1 = 0;
    uint64_t end_address_1 = 10;
    ModuleInfo module_info1;
    module_info1.set_file_path(file_path_1);
    module_info1.set_address_start(start_address1);
    module_info1.set_address_end(end_address_1);

    const std::string file_path_2 = "filepath2";
    uint64_t start_address_2 = 100;
    uint64_t end_address_2 = 110;
    ModuleInfo module_info2;
    module_info2.set_file_path(file_path_2);
    module_info2.set_address_start(start_address_2);
    module_info2.set_address_end(end_address_2);

    std::vector<ModuleInfo> module_infos{module_info1, module_info2};

    ProcessData process(ProcessInfo{});
    process.UpdateModuleInfos(module_infos);

    const absl::flat_hash_map<std::string, MemorySpace>& module_memory_map = process.GetMemoryMap();

    EXPECT_EQ(module_memory_map.size(), 2);

    const MemorySpace& memory_space_1 = module_memory_map.at(file_path_1);
    EXPECT_EQ(memory_space_1.start, start_address1);
    EXPECT_EQ(memory_space_1.end, end_address_1);
    const MemorySpace& memory_space_2 = module_memory_map.at(file_path_2);
    EXPECT_EQ(memory_space_2.start, start_address_2);
    EXPECT_EQ(memory_space_2.end, end_address_2);
  }
  {
    // invalid module infos: same filepath
    const std::string file_path = "file/path";

    uint64_t start_address_1 = 0;
    uint64_t end_address_1 = 10;
    ModuleInfo module_info_1;
    module_info_1.set_file_path(file_path);
    module_info_1.set_address_start(start_address_1);
    module_info_1.set_address_end(end_address_1);

    uint64_t start_address_2 = 100;
    uint64_t end_address_2 = 110;
    ModuleInfo module_info_2;
    module_info_2.set_file_path(file_path);
    module_info_2.set_address_start(start_address_2);
    module_info_1.set_address_end(end_address_2);

    std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

    ProcessData process;
    ASSERT_DEATH(process.UpdateModuleInfos(module_infos), "Check failed");
  }
  {
    // invalid module infos: same start address
    uint64_t start_address = 0;

    const std::string file_path_1 = "filepath1";
    uint64_t end_address_1 = 10;
    ModuleInfo module_info_1;
    module_info_1.set_file_path(file_path_1);
    module_info_1.set_address_start(start_address);
    module_info_1.set_address_end(end_address_1);

    const std::string file_path_2 = "filepath2";
    uint64_t end_address_2 = 110;
    ModuleInfo module_info_2;
    module_info_2.set_file_path(file_path_2);
    module_info_2.set_address_start(start_address);
    module_info_1.set_address_end(end_address_2);

    std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

    ProcessData process(ProcessInfo{});
    ASSERT_DEATH(process.UpdateModuleInfos(module_infos), "Check failed");
  }
}

TEST(ProcessData, MemorySpace) {
  {
    // AddressRange
    uint64_t start = 0x4000;
    uint64_t end = 0x4100;
    MemorySpace ms{start, end};
    EXPECT_EQ(ms.FormattedAddressRange(), "[0000000000004000 - 0000000000004100]");
  }
}

TEST(ProcessData, IsModuleLoaded) {
  const std::string file_path_1 = "filepath1";
  uint64_t start_address_1 = 0;
  uint64_t end_address_1 = 10;
  ModuleInfo module_info_1;
  module_info_1.set_file_path(file_path_1);
  module_info_1.set_address_start(start_address_1);
  module_info_1.set_address_end(end_address_1);

  const std::string file_path_2 = "filepath2";
  uint64_t start_address_2 = 100;
  uint64_t end_address_2 = 110;
  ModuleInfo module_info_2;
  module_info_2.set_file_path(file_path_2);
  module_info_2.set_address_start(start_address_2);
  module_info_1.set_address_end(end_address_2);

  std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos(module_infos);

  EXPECT_TRUE(process.IsModuleLoaded(file_path_1));
  EXPECT_TRUE(process.IsModuleLoaded(file_path_2));
  EXPECT_FALSE(process.IsModuleLoaded("not/loaded/module"));
}

TEST(ProcessData, GetModuleBaseAddress) {
  const std::string file_path_1 = "filepath1";
  uint64_t start_address_1 = 0;
  uint64_t end_address_1 = 10;
  ModuleInfo module_info_1;
  module_info_1.set_file_path(file_path_1);
  module_info_1.set_address_start(start_address_1);
  module_info_1.set_address_end(end_address_1);

  const std::string file_path_2 = "filepath2";
  uint64_t start_address_2 = 100;
  uint64_t end_address_2 = 110;
  ModuleInfo module_info_2;
  module_info_2.set_file_path(file_path_2);
  module_info_2.set_address_start(start_address_2);
  module_info_1.set_address_end(end_address_2);

  std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos(module_infos);

  EXPECT_EQ(process.GetModuleBaseAddress(file_path_1), start_address_1);
  EXPECT_EQ(process.GetModuleBaseAddress(file_path_2), start_address_2);
  EXPECT_DEATH(process.GetModuleBaseAddress("does/not/exist"), "Check failed");
}

TEST(ProcessData, CreateCopy) {
  const std::string process_name = "Test Name";
  const std::string module_path = "test/file/path";
  uint64_t start_address = 0x100;

  ProcessInfo info;
  info.set_name(process_name);
  ProcessData process(info);

  ModuleInfo module_info;
  module_info.set_file_path(module_path);
  module_info.set_address_start(start_address);

  process.UpdateModuleInfos({module_info});

  ProcessData process_copy = process.CreateCopy();

  EXPECT_EQ(process_copy.name(), process_name);
  EXPECT_TRUE(process_copy.IsModuleLoaded(module_path));
  ASSERT_EQ(process_copy.GetMemoryMap().size(), 1);
  EXPECT_EQ(process_copy.GetMemoryMap().at(module_path).start, start_address);
}

TEST(ProcessData, FindModuleByAddress) {
  const std::string process_name = "Test Name";
  const std::string module_path = "test/file/path";
  uint64_t start_address = 100;
  uint64_t end_address = 200;

  ProcessInfo info;
  info.set_name(process_name);
  ProcessData process(info);

  {
    // no modules loaded yet
    const auto result = process.FindModuleByAddress(0);
    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to find module for address"));
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no modules loaded"));
  }

  ModuleInfo module_info;
  module_info.set_file_path(module_path);
  module_info.set_address_start(start_address);
  module_info.set_address_end(end_address);

  process.UpdateModuleInfos({module_info});

  {
    // before start address
    const auto result = process.FindModuleByAddress(start_address - 10);
    EXPECT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to find module for address"));
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no module loaded at this address"));
  }
  {
    // start address
    const auto result = process.FindModuleByAddress(start_address);
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value().first, module_path);
    EXPECT_EQ(result.value().second, start_address);
  }
  {
    // after start address
    const auto result = process.FindModuleByAddress(start_address + 10);
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value().first, module_path);
    EXPECT_EQ(result.value().second, start_address);
  }
  {
    // exactly end address
    const auto result = process.FindModuleByAddress(end_address);
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value().first, module_path);
    EXPECT_EQ(result.value().second, start_address);
  }
  {
    // after end address
    const auto result = process.FindModuleByAddress(end_address + 10);
    EXPECT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to find module for address"));
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no module loaded at this address"));
  }
}
