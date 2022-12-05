// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/ascii.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_symbol_provider::ModuleIdentifier;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

using testing::AllOf;
using testing::ElementsAre;
using testing::Property;
using testing::UnorderedElementsAre;

namespace orbit_client_data {

TEST(ProcessData, Constructor) {
  uint32_t pid = 10;
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
    constexpr const char* kBuildId1 = "build_id_1";
    constexpr const char* kBuildId2 = "build_id_2";
    uint64_t start_address1 = 0;
    uint64_t end_address_1 = 10;
    ModuleInfo module_info1;
    module_info1.set_file_path(file_path_1);
    module_info1.set_build_id(kBuildId1);
    module_info1.set_address_start(start_address1);
    module_info1.set_address_end(end_address_1);

    const std::string file_path_2 = "filepath2";
    uint64_t start_address_2 = 100;
    uint64_t end_address_2 = 110;
    ModuleInfo module_info2;
    module_info2.set_file_path(file_path_2);
    module_info2.set_build_id(kBuildId2);
    module_info2.set_address_start(start_address_2);
    module_info2.set_address_end(end_address_2);

    std::vector<ModuleInfo> module_infos{module_info1, module_info2};

    ProcessData process(ProcessInfo{});
    process.UpdateModuleInfos(module_infos);

    const std::map<uint64_t, ModuleInMemory> module_memory_map = process.GetMemoryMapCopy();

    EXPECT_EQ(module_memory_map.size(), 2);

    const ModuleInMemory& memory_space_1 = module_memory_map.at(start_address1);
    EXPECT_EQ(memory_space_1.start(), start_address1);
    EXPECT_EQ(memory_space_1.end(), end_address_1);
    EXPECT_EQ(memory_space_1.file_path(), file_path_1);
    EXPECT_EQ(memory_space_1.build_id(), kBuildId1);

    const ModuleInMemory& memory_space_2 = module_memory_map.at(start_address_2);
    EXPECT_EQ(memory_space_2.start(), start_address_2);
    EXPECT_EQ(memory_space_2.end(), end_address_2);
    EXPECT_EQ(memory_space_2.file_path(), file_path_2);
    EXPECT_EQ(memory_space_2.build_id(), kBuildId2);
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
    ModuleInMemory ms{start, end, "path/to/file", "build_id"};
    EXPECT_EQ(ms.FormattedAddressRange(), "[0000000000004000 - 0000000000004100]");
  }
}

TEST(ProcessData, FindModuleBuildIdsByPath) {
  constexpr const char* kFilePath1 = "filepath1";
  constexpr const char* kBuildId1 = "buildid1";
  constexpr uint64_t kStartAddress1 = 0;
  constexpr uint64_t kEndAddress1 = 10;
  ModuleInfo module_info_1;
  module_info_1.set_file_path(kFilePath1);
  module_info_1.set_build_id(kBuildId1);
  module_info_1.set_address_start(kStartAddress1);
  module_info_1.set_address_end(kEndAddress1);

  constexpr const char* kFilePath2 = "filepath2";
  constexpr const char* kBuildId2 = "buildid2";
  constexpr uint64_t kStartAddress2 = 100;
  constexpr uint64_t kEndAddress2 = 110;
  ModuleInfo module_info_2;
  module_info_2.set_file_path(kFilePath2);
  module_info_2.set_build_id(kBuildId2);
  module_info_2.set_address_start(kStartAddress2);
  module_info_2.set_address_end(kEndAddress2);

  constexpr const char* kFilePath3 = kFilePath2;
  constexpr const char* kBuildId3 = "buildid3";
  constexpr uint64_t kStartAddress3 = 200;
  constexpr uint64_t kEndAddress3 = 210;
  ModuleInfo module_info_3;
  module_info_3.set_file_path(kFilePath3);
  module_info_3.set_build_id(kBuildId3);
  module_info_3.set_address_start(kStartAddress3);
  module_info_3.set_address_end(kEndAddress3);

  std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos(module_infos);
  process.AddOrUpdateModuleInfo(module_info_3);

  EXPECT_TRUE(process.IsModuleLoadedByProcess(kFilePath1));
  EXPECT_THAT(process.FindModuleBuildIdsByPath(kFilePath1), ElementsAre(kBuildId1));
  EXPECT_TRUE(process.IsModuleLoadedByProcess(kFilePath2));
  EXPECT_THAT(process.FindModuleBuildIdsByPath(kFilePath2), ElementsAre(kBuildId2, kBuildId3));
  EXPECT_TRUE(process.IsModuleLoadedByProcess(kFilePath3));
  EXPECT_THAT(process.FindModuleBuildIdsByPath(kFilePath3), ElementsAre(kBuildId2, kBuildId3));
  EXPECT_FALSE(process.IsModuleLoadedByProcess("not/loaded/module"));
  EXPECT_TRUE(process.FindModuleBuildIdsByPath("not/loaded/module").empty());
}

TEST(ProcessData, FindModulesByFilename) {
  constexpr const char* kFileName1 = "file1";
  constexpr const char* kFilePath1 = "path/to/file1";
  constexpr const char* kBuildId1 = "buildid1";
  constexpr uint64_t kStartAddress1 = 0;
  constexpr uint64_t kEndAddress1 = 10;
  ModuleInfo module_info_1;
  module_info_1.set_file_path(kFilePath1);
  module_info_1.set_build_id(kBuildId1);
  module_info_1.set_address_start(kStartAddress1);
  module_info_1.set_address_end(kEndAddress1);

  constexpr const char* kFileName2 = "file2";
  constexpr const char* kFilePath2 = "path/to/file2";
  constexpr const char* kBuildId2 = "buildid2";
  constexpr uint64_t kStartAddress2 = 100;
  constexpr uint64_t kEndAddress2 = 110;
  ModuleInfo module_info_2;
  module_info_2.set_file_path(kFilePath2);
  module_info_2.set_build_id(kBuildId2);
  module_info_2.set_address_start(kStartAddress2);
  module_info_2.set_address_end(kEndAddress2);

  constexpr const char* kBuildId3 = "kBuildId3";
  constexpr uint64_t kStartAddress3 = 200;
  constexpr uint64_t kEndAddress3 = 210;
  ModuleInfo module_info_3;
  module_info_3.set_file_path(kFilePath2);
  module_info_3.set_build_id(kBuildId3);
  module_info_3.set_address_start(kStartAddress3);
  module_info_3.set_address_end(kEndAddress3);

  constexpr uint64_t kStartAddress4 = 300;
  constexpr uint64_t kEndAddress4 = 310;
  ModuleInfo module_info_4;
  module_info_4.set_file_path(kFilePath2);
  module_info_4.set_build_id(kBuildId2);
  module_info_4.set_address_start(kStartAddress4);
  module_info_4.set_address_end(kEndAddress4);

  std::vector<ModuleInfo> module_infos{module_info_1, module_info_2, module_info_3, module_info_4};

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos(module_infos);

  EXPECT_THAT(process.FindModulesByFilename(kFileName1),
              UnorderedElementsAre(AllOf(Property(&ModuleInMemory::file_path, kFilePath1),
                                         Property(&ModuleInMemory::build_id, kBuildId1),
                                         Property(&ModuleInMemory::start, kStartAddress1),
                                         Property(&ModuleInMemory::end, kEndAddress1))));

  EXPECT_THAT(process.FindModulesByFilename(kFileName2),
              UnorderedElementsAre(AllOf(Property(&ModuleInMemory::file_path, kFilePath2),
                                         Property(&ModuleInMemory::build_id, kBuildId2),
                                         Property(&ModuleInMemory::start, kStartAddress2),
                                         Property(&ModuleInMemory::end, kEndAddress2)),
                                   AllOf(Property(&ModuleInMemory::file_path, kFilePath2),
                                         Property(&ModuleInMemory::build_id, kBuildId3),
                                         Property(&ModuleInMemory::start, kStartAddress3),
                                         Property(&ModuleInMemory::end, kEndAddress3)),
                                   AllOf(Property(&ModuleInMemory::file_path, kFilePath2),
                                         Property(&ModuleInMemory::build_id, kBuildId2),
                                         Property(&ModuleInMemory::start, kStartAddress4),
                                         Property(&ModuleInMemory::end, kEndAddress4))));
}

TEST(ProcessData, IsModuleLoadedByProcess) {
  ModuleInfo module_info_1;
  module_info_1.set_file_path("path/to/file1");
  module_info_1.set_address_start(0);
  module_info_1.set_address_end(10);

  ModuleInfo module_info_2;
  module_info_2.set_file_path("path/to/file2");
  module_info_2.set_address_start(100);
  module_info_2.set_address_end(110);
  module_info_2.set_build_id("build_id_2");

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos({module_info_1, module_info_2});

  // path empty
  EXPECT_FALSE(process.IsModuleLoadedByProcess(""));

  // wrong path
  EXPECT_FALSE(process.IsModuleLoadedByProcess("/path/to/file1"));

  // correct path
  EXPECT_TRUE(process.IsModuleLoadedByProcess("path/to/file1"));

  // Module without build id
  ModuleData module_1{module_info_1};
  EXPECT_TRUE(process.IsModuleLoadedByProcess(&module_1));

  // Module with build id
  ModuleData module_2{module_info_2};
  EXPECT_TRUE(process.IsModuleLoadedByProcess(&module_2));

  // Different module (same path, different build_id)
  ModuleInfo module_info_3;
  module_info_3.set_file_path("path/to/file1");
  module_info_3.set_address_start(0);
  module_info_3.set_address_end(10);
  module_info_3.set_build_id("build_id_3");
  process.AddOrUpdateModuleInfo({module_info_3});

  EXPECT_TRUE(process.IsModuleLoadedByProcess("path/to/file1"));
  EXPECT_FALSE(process.IsModuleLoadedByProcess(&module_1));
  ModuleData module_3{module_info_3};
  EXPECT_TRUE(process.IsModuleLoadedByProcess(&module_3));
}

TEST(ProcessData, GetModuleBaseAddresses) {
  const std::string file_path_1 = "filepath1";
  const std::string build_id_1 = "buildid1";
  uint64_t start_address_1 = 0;
  uint64_t end_address_1 = 10;
  ModuleInfo module_info_1;
  module_info_1.set_file_path(file_path_1);
  module_info_1.set_build_id(build_id_1);
  module_info_1.set_address_start(start_address_1);
  module_info_1.set_address_end(end_address_1);

  const std::string file_path_2 = "filepath2";
  const std::string build_id_2 = "buildid2";
  uint64_t start_address_2 = 100;
  uint64_t end_address_2 = 110;
  ModuleInfo module_info_2;
  module_info_2.set_file_path(file_path_2);
  module_info_2.set_build_id(build_id_2);
  module_info_2.set_address_start(start_address_2);
  module_info_2.set_address_end(end_address_2);

  const std::string& file_path_3 = file_path_2;
  const std::string& build_id_3 = build_id_2;
  uint64_t start_address_3 = 300;
  uint64_t end_address_3 = 310;
  ModuleInfo module_info_3;
  module_info_3.set_file_path(file_path_3);
  module_info_3.set_build_id(build_id_3);
  module_info_3.set_address_start(start_address_3);
  module_info_3.set_address_end(end_address_3);

  std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos(module_infos);

  {
    const std::vector<uint64_t> file_1_base_address =
        process.GetModuleBaseAddresses(file_path_1, build_id_1);
    ASSERT_EQ(file_1_base_address.size(), 1);
    EXPECT_THAT(file_1_base_address, ElementsAre(start_address_1));

    const std::vector<uint64_t> file_2_base_address =
        process.GetModuleBaseAddresses(file_path_2, build_id_2);
    ASSERT_EQ(file_2_base_address.size(), 1);
    EXPECT_THAT(file_2_base_address, ElementsAre(start_address_2));

    EXPECT_EQ(process.GetModuleBaseAddresses("does/not/exist", "nobuildid").size(), 0);
    EXPECT_EQ(process.GetModuleBaseAddresses(file_path_1, build_id_2).size(), 0);
  }

  process.AddOrUpdateModuleInfo(module_info_3);

  {
    const std::vector<uint64_t> file_1_base_address =
        process.GetModuleBaseAddresses(file_path_1, build_id_1);
    ASSERT_EQ(file_1_base_address.size(), 1);
    EXPECT_THAT(file_1_base_address, ElementsAre(start_address_1));

    const std::vector<uint64_t> file_2_base_address =
        process.GetModuleBaseAddresses(file_path_2, build_id_2);
    ASSERT_EQ(file_2_base_address.size(), 2);
    EXPECT_THAT(file_2_base_address, ElementsAre(start_address_2, start_address_3));

    EXPECT_EQ(process.GetModuleBaseAddresses("does/not/exist", "nobuildid").size(), 0);
    EXPECT_EQ(process.GetModuleBaseAddresses(file_path_1, build_id_2).size(), 0);
  }
}

TEST(ProcessData, FindModuleByAddress) {
  const std::string process_name = "Test Name";
  const std::string module_path = "test/file/path";
  constexpr const char* kBuildId = "42";
  uint64_t start_address = 100;
  uint64_t end_address = 200;

  ProcessInfo info;
  info.set_name(process_name);
  ProcessData process(info);

  {
    // no modules loaded yet
    const auto result = process.FindModuleByAddress(0);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to find module for address"));
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no modules loaded"));
  }

  ModuleInfo module_info;
  module_info.set_file_path(module_path);
  module_info.set_build_id(kBuildId);
  module_info.set_address_start(start_address);
  module_info.set_address_end(end_address);

  process.UpdateModuleInfos({module_info});

  {
    // before start address
    const auto result = process.FindModuleByAddress(start_address - 10);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to find module for address"));
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no module loaded at this address"));
  }
  {
    // start address
    const auto result = process.FindModuleByAddress(start_address);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().file_path(), module_path);
    EXPECT_EQ(result.value().start(), start_address);
    EXPECT_EQ(result.value().end(), end_address);
    EXPECT_EQ(result.value().build_id(), kBuildId);
  }
  {
    // after start address
    const auto result = process.FindModuleByAddress(start_address + 10);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().file_path(), module_path);
    EXPECT_EQ(result.value().start(), start_address);
    EXPECT_EQ(result.value().end(), end_address);
    EXPECT_EQ(result.value().build_id(), kBuildId);
  }
  {
    // exactly end address
    const auto result = process.FindModuleByAddress(end_address);
    EXPECT_THAT(result, HasError("Unable to find module for address"));
    EXPECT_THAT(result, HasError("No module loaded at this address"));
  }
  {
    // after end address
    const auto result = process.FindModuleByAddress(end_address + 10);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to find module for address"));
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no module loaded at this address"));
  }
}

TEST(ProcessData, GetUniqueModulesPathAndBuildIds) {
  const std::string file_path_1 = "filepath1";
  const std::string build_id_1 = "build_id1";
  uint64_t start_address_1 = 0;
  uint64_t end_address_1 = 10;
  ModuleInfo module_info_1;
  module_info_1.set_file_path(file_path_1);
  module_info_1.set_build_id(build_id_1);
  module_info_1.set_address_start(start_address_1);
  module_info_1.set_address_end(end_address_1);

  const std::string file_path_2 = "filepath2";
  const std::string build_id_2 = "build_id2";
  uint64_t start_address_2 = 100;
  uint64_t end_address_2 = 110;
  ModuleInfo module_info_2;
  module_info_2.set_file_path(file_path_2);
  module_info_2.set_build_id(build_id_2);
  module_info_2.set_address_start(start_address_2);
  module_info_2.set_address_end(end_address_2);

  const std::string& file_path_3 = file_path_2;
  const std::string& build_id_3 = build_id_2;
  uint64_t start_address_3 = 400;
  uint64_t end_address_3 = 410;
  ModuleInfo module_info_3;
  module_info_3.set_file_path(file_path_3);
  module_info_3.set_build_id(build_id_3);
  module_info_3.set_address_start(start_address_3);
  module_info_3.set_address_end(end_address_3);

  std::vector<ModuleInfo> module_infos{module_info_1, module_info_2};

  ProcessData process(ProcessInfo{});
  process.UpdateModuleInfos(module_infos);
  process.AddOrUpdateModuleInfo(module_info_3);

  auto module_ids = process.GetUniqueModuleIdentifiers();
  ASSERT_EQ(module_ids.size(), 2);
  EXPECT_THAT(module_ids, testing::UnorderedElementsAre(ModuleIdentifier{file_path_1, build_id_1},
                                                        ModuleIdentifier{file_path_2, build_id_2}));
}

TEST(ProcessData, RemapModule) {
  constexpr const char* kProcessName = "Test Name";
  constexpr const char* kModulePath = "test/file/path";
  constexpr const char* kBuildId = "42";
  constexpr uint64_t kStartAddress = 100;
  constexpr uint64_t kEndAddress = 200;

  constexpr uint64_t kNewStartAddress = 300;
  constexpr uint64_t kNewEndAddress = 400;

  ProcessInfo info;
  info.set_name(kProcessName);
  ProcessData process(info);

  EXPECT_THAT(process.FindModuleByAddress(0), HasError("Unable to find module for address"));

  ModuleInfo module_info;
  module_info.set_file_path(kModulePath);
  module_info.set_build_id(kBuildId);
  module_info.set_address_start(kStartAddress);
  module_info.set_address_end(kEndAddress);

  process.UpdateModuleInfos({module_info});

  {
    const auto result = process.FindModuleByAddress(kStartAddress);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath);
    EXPECT_EQ(result.value().start(), kStartAddress);
    EXPECT_EQ(result.value().end(), kEndAddress);
    EXPECT_EQ(result.value().build_id(), kBuildId);
  }

  module_info.set_address_start(kNewStartAddress);
  module_info.set_address_end(kNewEndAddress);
  process.AddOrUpdateModuleInfo(module_info);

  {
    // old start address is still there and has correct data
    const auto result = process.FindModuleByAddress(kStartAddress);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath);
    EXPECT_EQ(result.value().start(), kStartAddress);
    EXPECT_EQ(result.value().end(), kEndAddress);
    EXPECT_EQ(result.value().build_id(), kBuildId);
  }

  {
    // new start address is also available
    const auto result = process.FindModuleByAddress(kNewStartAddress);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath);
    EXPECT_EQ(result.value().start(), kNewStartAddress);
    EXPECT_EQ(result.value().end(), kNewEndAddress);
    EXPECT_EQ(result.value().build_id(), kBuildId);
  }
}

class ProcessDataModuleIntersectionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ProcessInfo info;
    info.set_name(kProcessName);
    process_.SetProcessInfo(info);
    process_.UpdateModuleInfos(initial_mapping_);
  }

  static constexpr const char* kProcessName = "Test Name";

  static constexpr const char* kModulePath0 = "test/file/path0";
  static constexpr const char* kBuildId0 = "build_id0";
  static constexpr uint64_t kStartAddress0 = 50;
  static constexpr uint64_t kEndAddress0 = 100;

  static constexpr const char* kModulePath1 = "test/file/path1";
  static constexpr const char* kBuildId1 = "build_id1";
  static constexpr uint64_t kStartAddress1 = 100;
  static constexpr uint64_t kEndAddress1 = 200;

  static constexpr const char* kModulePath2 = "test/file/path2";
  static constexpr const char* kBuildId2 = "build_id2";
  static constexpr uint64_t kStartAddress2 = 200;
  static constexpr uint64_t kEndAddress2 = 300;

  static constexpr const char* kModulePath3 = "test/file/path3";
  static constexpr const char* kBuildId3 = "build_id3";
  static constexpr uint64_t kStartAddress3 = 300;
  static constexpr uint64_t kEndAddress3 = 400;

  static constexpr const char* kNewModulePath = "test/file/path";
  static constexpr const char* kNewBuildId = "build_id";

  static ModuleInfo CreateModule(std::string module_path, std::string build_id,
                                 uint64_t start_address, uint64_t end_address) {
    ModuleInfo module_info;
    module_info.set_file_path(std::move(module_path));
    module_info.set_build_id(std::move(build_id));
    module_info.set_address_start(start_address);
    module_info.set_address_end(end_address);
    return module_info;
  }

  const std::vector<ModuleInfo> initial_mapping_{
      CreateModule(kModulePath0, kBuildId0, kStartAddress0, kEndAddress0),
      CreateModule(kModulePath1, kBuildId1, kStartAddress1, kEndAddress1),
      CreateModule(kModulePath2, kBuildId2, kStartAddress2, kEndAddress2),
      CreateModule(kModulePath3, kBuildId3, kStartAddress3, kEndAddress3)};

  ProcessData process_;
};

TEST_F(ProcessDataModuleIntersectionTest, IntersectWithTwoModules) {
  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 4);

  ModuleInfo intersecting_module = CreateModule(kNewModulePath, kNewBuildId, 150, 250);
  process_.AddOrUpdateModuleInfo(intersecting_module);

  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 3);

  // Non intersecting modules are still there
  {
    const auto result = process_.FindModuleByAddress(kStartAddress0);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath0);
    EXPECT_EQ(result.value().build_id(), kBuildId0);
    EXPECT_EQ(result.value().start(), kStartAddress0);
    EXPECT_EQ(result.value().end(), kEndAddress0);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress3);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath3);
    EXPECT_EQ(result.value().build_id(), kBuildId3);
    EXPECT_EQ(result.value().start(), kStartAddress3);
    EXPECT_EQ(result.value().end(), kEndAddress3);
  }

  {
    // We can find the new module
    const auto result = process_.FindModuleByAddress(150);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kNewModulePath);
    EXPECT_EQ(result.value().build_id(), kNewBuildId);
    EXPECT_EQ(result.value().start(), 150);
    EXPECT_EQ(result.value().end(), 250);

    // Intersecting modules are gone
    EXPECT_THAT(process_.FindModuleByAddress(148), HasError("Unable to find module for address"));
    EXPECT_THAT(process_.FindModuleByAddress(250), HasError("Unable to find module for address"));
    EXPECT_THAT(process_.FindModuleByAddress(270), HasError("Unable to find module for address"));
  }
}

TEST_F(ProcessDataModuleIntersectionTest, IntersectWithTwoModulesWithMatchingBorders) {
  ModuleInfo intersecting_module = CreateModule(kNewModulePath, kNewBuildId, 100, 300);
  process_.AddOrUpdateModuleInfo(intersecting_module);

  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 3);

  // Non intersecting modules are still there
  {
    const auto result = process_.FindModuleByAddress(kStartAddress0);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath0);
    EXPECT_EQ(result.value().build_id(), kBuildId0);
    EXPECT_EQ(result.value().start(), kStartAddress0);
    EXPECT_EQ(result.value().end(), kEndAddress0);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress3);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath3);
    EXPECT_EQ(result.value().build_id(), kBuildId3);
    EXPECT_EQ(result.value().start(), kStartAddress3);
    EXPECT_EQ(result.value().end(), kEndAddress3);
  }

  {
    // We can find the new module
    const auto result = process_.FindModuleByAddress(150);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kNewModulePath);
    EXPECT_EQ(result.value().build_id(), kNewBuildId);
    EXPECT_EQ(result.value().start(), 100);
    EXPECT_EQ(result.value().end(), 300);
  }
}

TEST_F(ProcessDataModuleIntersectionTest, FullyInsideAnotherModuleAddressRange) {
  // Address range is fully inside another module's address range
  ModuleInfo intersecting_module = CreateModule(kNewModulePath, kNewBuildId, 110, 190);
  process_.AddOrUpdateModuleInfo(intersecting_module);

  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 4);

  // Non intersecting modules are still there
  {
    const auto result = process_.FindModuleByAddress(kStartAddress0);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath0);
    EXPECT_EQ(result.value().build_id(), kBuildId0);
    EXPECT_EQ(result.value().start(), kStartAddress0);
    EXPECT_EQ(result.value().end(), kEndAddress0);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress2);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath2);
    EXPECT_EQ(result.value().build_id(), kBuildId2);
    EXPECT_EQ(result.value().start(), kStartAddress2);
    EXPECT_EQ(result.value().end(), kEndAddress2);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress3);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath3);
    EXPECT_EQ(result.value().build_id(), kBuildId3);
    EXPECT_EQ(result.value().start(), kStartAddress3);
    EXPECT_EQ(result.value().end(), kEndAddress3);
  }

  {
    // We can find the new module
    const auto result = process_.FindModuleByAddress(150);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kNewModulePath);
    EXPECT_EQ(result.value().build_id(), kNewBuildId);
    EXPECT_EQ(result.value().start(), 110);
    EXPECT_EQ(result.value().end(), 190);

    // Original module is gone
    EXPECT_THAT(process_.FindModuleByAddress(108), HasError("Unable to find module for address"));
    EXPECT_THAT(process_.FindModuleByAddress(190), HasError("Unable to find module for address"));
  }
}

TEST_F(ProcessDataModuleIntersectionTest, OverlapsWithEverything) {
  ModuleInfo intersecting_module = CreateModule(kNewModulePath, kNewBuildId, 10, 450);
  process_.AddOrUpdateModuleInfo(intersecting_module);

  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 1);

  {
    const auto result = process_.FindModuleByAddress(kStartAddress0);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kNewModulePath);
    EXPECT_EQ(result.value().build_id(), kNewBuildId);
    EXPECT_EQ(result.value().start(), 10);
    EXPECT_EQ(result.value().end(), 450);
  }
}

TEST_F(ProcessDataModuleIntersectionTest, ReplaceFirstModule) {
  ModuleInfo intersecting_module = CreateModule(kNewModulePath, kNewBuildId, 10, 90);
  process_.AddOrUpdateModuleInfo(intersecting_module);

  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 4);

  {
    const auto result = process_.FindModuleByAddress(50);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kNewModulePath);
    EXPECT_EQ(result.value().build_id(), kNewBuildId);
    EXPECT_EQ(result.value().start(), 10);
    EXPECT_EQ(result.value().end(), 90);

    // Original module is gone
    EXPECT_THAT(process_.FindModuleByAddress(90), HasError("Unable to find module for address"));
  }

  // Non intersecting modules are still there
  {
    const auto result = process_.FindModuleByAddress(kStartAddress1);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath1);
    EXPECT_EQ(result.value().build_id(), kBuildId1);
    EXPECT_EQ(result.value().start(), kStartAddress1);
    EXPECT_EQ(result.value().end(), kEndAddress1);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress2);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath2);
    EXPECT_EQ(result.value().build_id(), kBuildId2);
    EXPECT_EQ(result.value().start(), kStartAddress2);
    EXPECT_EQ(result.value().end(), kEndAddress2);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress3);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath3);
    EXPECT_EQ(result.value().build_id(), kBuildId3);
    EXPECT_EQ(result.value().start(), kStartAddress3);
    EXPECT_EQ(result.value().end(), kEndAddress3);
  }
}

TEST_F(ProcessDataModuleIntersectionTest, ReplaceLastModule) {
  ModuleInfo intersecting_module = CreateModule(kNewModulePath, kNewBuildId, 350, 450);
  process_.AddOrUpdateModuleInfo(intersecting_module);

  EXPECT_EQ(process_.GetMemoryMapCopy().size(), 4);

  {
    const auto result = process_.FindModuleByAddress(370);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kNewModulePath);
    EXPECT_EQ(result.value().build_id(), kNewBuildId);
    EXPECT_EQ(result.value().start(), 350);
    EXPECT_EQ(result.value().end(), 450);

    // Original module is gone
    EXPECT_THAT(process_.FindModuleByAddress(310), HasError("Unable to find module for address"));
  }

  // Non intersecting modules are still there
  {
    const auto result = process_.FindModuleByAddress(kStartAddress0);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath0);
    EXPECT_EQ(result.value().build_id(), kBuildId0);
    EXPECT_EQ(result.value().start(), kStartAddress0);
    EXPECT_EQ(result.value().end(), kEndAddress0);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress1);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath1);
    EXPECT_EQ(result.value().build_id(), kBuildId1);
    EXPECT_EQ(result.value().start(), kStartAddress1);
    EXPECT_EQ(result.value().end(), kEndAddress1);
  }

  {
    const auto result = process_.FindModuleByAddress(kStartAddress2);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().file_path(), kModulePath2);
    EXPECT_EQ(result.value().build_id(), kBuildId2);
    EXPECT_EQ(result.value().start(), kStartAddress2);
    EXPECT_EQ(result.value().end(), kEndAddress2);
  }
}
}  // namespace orbit_client_data
