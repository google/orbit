// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <unistd.h>

#include <filesystem>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "module.pb.h"

TEST(LinuxMap, CreateModuleHelloWorld) {
  using orbit_elf_utils::CreateModuleFromFile;
  using orbit_grpc_protos::ModuleInfo;

  const std::filesystem::path hello_world_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModuleFromFile(hello_world_path, kStartAddress, kEndAddress);
  ASSERT_FALSE(result.has_error()) << result.error().message();

  EXPECT_EQ(result.value().name(), "hello_world_elf");
  EXPECT_EQ(result.value().file_path(), hello_world_path);
  EXPECT_EQ(result.value().file_size(), 16616);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().build_id(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
  EXPECT_EQ(result.value().load_bias(), 0x0);
}

TEST(LinuxMap, CreateModuleOnDev) {
  using orbit_elf_utils::CreateModuleFromFile;
  using orbit_grpc_protos::ModuleInfo;

  const std::filesystem::path dev_zero_path = "/dev/zero";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModuleFromFile(dev_zero_path, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_EQ(result.error().message(),
            "The module \"/dev/zero\" is a character or block device (is in /dev/)");
}

TEST(LinuxMap, CreateModuleNotElf) {
  using orbit_elf_utils::CreateModuleFromFile;
  using orbit_grpc_protos::ModuleInfo;

  const std::filesystem::path text_file =
      orbit_base::GetExecutableDir() / "testdata" / "textfile.txt";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModuleFromFile(text_file, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_THAT(result.error().message(),
              testing::HasSubstr("The file was not recognized as a valid object file"));
}

TEST(LinuxMan, CreateModuleWithSoname) {
  using orbit_elf_utils::CreateModuleFromFile;
  using orbit_grpc_protos::ModuleInfo;

  const std::filesystem::path hello_world_path =
      orbit_base::GetExecutableDir() / "testdata" / "libtest-1.0.so";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModuleFromFile(hello_world_path, kStartAddress, kEndAddress);
  ASSERT_FALSE(result.has_error()) << result.error().message();

  EXPECT_EQ(result.value().name(), "libtest.so");
  EXPECT_EQ(result.value().file_path(), hello_world_path);
  EXPECT_EQ(result.value().file_size(), 16128);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().build_id(), "2e70049c5cf42e6c5105825b57104af5882a40a2");
  EXPECT_EQ(result.value().load_bias(), 0x0);
}

TEST(LinuxMap, CreateModuleFileDoesNotExist) {
  using orbit_elf_utils::CreateModuleFromFile;
  using orbit_grpc_protos::ModuleInfo;

  const std::filesystem::path file_path = "/not/a/valid/file/path";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModuleFromFile(file_path, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_EQ(result.error().message(), "The module file \"/not/a/valid/file/path\" does not exist");
}

TEST(LinuxMap, ReadModules) {
  const auto result = orbit_elf_utils::ReadModules(getpid());
  EXPECT_FALSE(result.has_error()) << result.error().message();
}

TEST(LineMap, ParseMapEntryRegularFile) {
  const std::string map_line{
      "7f687428f000-7f6874290000 r-xp 00009000 fe:01 661216                     "
      "/whatever/file/path.so"};

  const std::optional<orbit_elf_utils::MapEntry> result = orbit_elf_utils::ParseMapEntry(map_line);

  ASSERT_TRUE(result.has_value());
  const auto& map_entry = result.value();

  EXPECT_EQ(map_entry.module_path, "/whatever/file/path.so");
  EXPECT_EQ(map_entry.start_address, 0x7f687428f000LU);
  EXPECT_EQ(map_entry.end_address, 0x7f6874290000LU);
  EXPECT_EQ(map_entry.inode, 661216);
  EXPECT_TRUE(map_entry.is_executable);
}

TEST(LineMap, ParseMapEntryVdso) {
  const std::string map_line{
      "7ffc6a78e000-7ffc6a790000 r-xp 00000000 00:00 0                          [vdso]"};

  const std::optional<orbit_elf_utils::MapEntry> result = orbit_elf_utils::ParseMapEntry(map_line);

  ASSERT_TRUE(result.has_value());
  const auto& map_entry = result.value();

  EXPECT_EQ(map_entry.module_path, "[vdso]");
  EXPECT_EQ(map_entry.start_address, 0x7ffc6a78e000LU);
  EXPECT_EQ(map_entry.end_address, 0x7ffc6a790000LU);
  EXPECT_EQ(map_entry.inode, 0);
  EXPECT_TRUE(map_entry.is_executable);
}

TEST(LinuxMap, ParseMaps) {
  using orbit_elf_utils::ParseMaps;
  using orbit_grpc_protos::ModuleInfo;

  {
    // Empty data
    const auto result = ParseMaps(std::string_view{""});
    EXPECT_TRUE(result.empty());
  }

  const std::filesystem::path test_path = orbit_base::GetExecutableDir() / "testdata";
  const std::filesystem::path hello_world_path = test_path / "hello_world_elf";
  const std::filesystem::path text_file = test_path / "textfile.txt";

  {
    const std::string data{absl::StrFormat(
        "7f687428f000-7f6874290000 r-xp 00009000 fe:01 661216                     "
        "/not/a/valid/file/path\n"
        "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     %s\n"
        "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     /dev/zero\n"
        "7f6874290001-7f6874297002 r-dp 00000000 fe:01 661214                     %s\n",
        hello_world_path, text_file)};
    const auto result = ParseMaps(data);
    EXPECT_EQ(result.size(), 4);
  }

  const std::filesystem::path no_symbols_path = test_path / "no_symbols_elf";
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

    const auto map_entries = ParseMaps(data);
    ASSERT_EQ(map_entries.size(), 2);  // Only two unique module paths

    std::vector<ModuleInfo> result;
    for (const auto& map_entry : map_entries) {
      auto module_or_error = orbit_elf_utils::CreateModuleFromFile(map_entry);
      if (module_or_error.has_value()) result.emplace_back(std::move(module_or_error.value()));
    }
    ASSERT_EQ(result.size(), 2);

    const ModuleInfo* hello_module_info = nullptr;
    const ModuleInfo* no_symbols_module_info = nullptr;

    if (result[0].name() == "hello_world_elf") {
      hello_module_info = &result[0];
      no_symbols_module_info = &result[1];
    } else {
      hello_module_info = &result[1];
      no_symbols_module_info = &result[0];
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

TEST(LinuxMap, CreateModuleFromBufferHelloWorld) {
  using orbit_elf_utils::CreateModuleFromBuffer;
  using orbit_grpc_protos::ModuleInfo;

  const std::filesystem::path hello_world_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";
  ASSERT_TRUE(std::filesystem::exists(hello_world_path));

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto buffer_or_error = orbit_base::ReadFileToString(hello_world_path);
  ASSERT_TRUE(buffer_or_error.has_value());
  const auto& buffer = buffer_or_error.value();

  auto result = CreateModuleFromBuffer(hello_world_path.filename().string(), buffer, kStartAddress,
                                       kEndAddress);
  ASSERT_FALSE(result.has_error()) << result.error().message();

  EXPECT_EQ(result.value().name(), "hello_world_elf");
  EXPECT_EQ(result.value().file_path(), "hello_world_elf");
  EXPECT_EQ(result.value().file_size(), 16616);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().build_id(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
  EXPECT_EQ(result.value().load_bias(), 0x0);
}