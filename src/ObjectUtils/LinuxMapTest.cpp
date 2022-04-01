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
#include <string>
#include <string_view>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_test_utils::HasNoError;

namespace orbit_object_utils {

TEST(LinuxMap, CreateModuleHelloWorld) {
  const std::filesystem::path hello_world_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(hello_world_path, kStartAddress, kEndAddress);
  ASSERT_THAT(result, HasNoError());

  EXPECT_EQ(result.value().name(), "hello_world_elf");
  EXPECT_EQ(result.value().file_path(), hello_world_path);
  EXPECT_EQ(result.value().file_size(), 16616);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().build_id(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
  EXPECT_EQ(result.value().load_bias(), 0x0);
  EXPECT_EQ(result.value().object_file_type(), ModuleInfo::kElfFile);
}

TEST(LinuxMap, CreateModuleOnDev) {
  const std::filesystem::path dev_zero_path = "/dev/zero";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(dev_zero_path, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_EQ(result.error().message(),
            "The module \"/dev/zero\" is a character or block device (is in /dev/)");
}

TEST(LinuxMap, CreateCoffModule) {
  const std::filesystem::path dll_path = orbit_test::GetTestdataDir() / "libtest.dll";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;

  auto result = CreateModule(dll_path, kStartAddress, kEndAddress);
  ASSERT_THAT(result, HasNoError());

  EXPECT_EQ(result.value().name(), "libtest.dll");
  EXPECT_EQ(result.value().file_path(), dll_path);
  EXPECT_EQ(result.value().file_size(), 96441);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().load_bias(), 0x62640000);
  EXPECT_EQ(result.value().executable_segment_offset(), 0x1000);
  EXPECT_EQ(result.value().build_id(), "");
  EXPECT_EQ(result.value().object_file_type(), ModuleInfo::kCoffFile);
}

TEST(LinuxMap, CreateModuleNotElf) {
  const std::filesystem::path text_file = orbit_test::GetTestdataDir() / "textfile.txt";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(text_file, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_THAT(result.error().message(),
              testing::HasSubstr("The file was not recognized as a valid object file"));
}

TEST(LinuxMap, CreateModuleWithSoname) {
  const std::filesystem::path hello_world_path = orbit_test::GetTestdataDir() / "libtest-1.0.so";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(hello_world_path, kStartAddress, kEndAddress);
  ASSERT_THAT(result, HasNoError());

  EXPECT_EQ(result.value().name(), "libtest.so");
  EXPECT_EQ(result.value().file_path(), hello_world_path);
  EXPECT_EQ(result.value().file_size(), 16128);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().build_id(), "2e70049c5cf42e6c5105825b57104af5882a40a2");
  EXPECT_EQ(result.value().load_bias(), 0x0);
  EXPECT_EQ(result.value().object_file_type(), ModuleInfo::kElfFile);
}

TEST(LinuxMap, CreateModuleFileDoesNotExist) {
  const std::filesystem::path file_path = "/not/a/valid/file/path";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(file_path, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_EQ(result.error().message(), "The module file \"/not/a/valid/file/path\" does not exist");
}

TEST(LinuxMap, ReadModules) {
  const auto result = ReadModules(getpid());
  EXPECT_THAT(result, HasNoError());
}

TEST(LinuxMap, ParseMaps) {
  {
    // Empty data
    const auto result = ParseMaps(std::string_view{""});
    ASSERT_THAT(result, HasNoError());
    EXPECT_TRUE(result.value().empty());
  }

  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path hello_world_path = test_path / "hello_world_elf";
  const std::filesystem::path text_file = test_path / "textfile.txt";

  {
    // Testing correct size of result. The entry with dev/zero is ignored due to the path starting
    // with /dev/. The last entry has a valid path, but the executable flag is not set.
    const std::string data{absl::StrFormat(
        "7f687428f000-7f6874290000 r-xp 00009000 fe:01 661216                     "
        "/not/a/valid/file/path\n"
        "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     %s\n"
        "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     /dev/zero\n"
        "7f6874290001-7f6874297002 r-dp 00000000 fe:01 661214                     %s\n",
        hello_world_path, text_file)};
    const auto result = ParseMaps(data);
    ASSERT_THAT(result, HasNoError());
    EXPECT_EQ(result.value().size(), 1);
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

    const auto result = ParseMaps(data);
    ASSERT_THAT(result, HasNoError());
    ASSERT_EQ(result.value().size(), 2);

    const ModuleInfo* hello_module_info = nullptr;
    const ModuleInfo* no_symbols_module_info = nullptr;

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
    EXPECT_EQ(hello_module_info->address_start(), 0x7f6874288000);
    EXPECT_EQ(hello_module_info->address_end(), 0x7f687428c000);
    EXPECT_EQ(hello_module_info->build_id(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
    EXPECT_EQ(hello_module_info->load_bias(), 0x0);
    EXPECT_EQ(hello_module_info->object_file_type(), ModuleInfo::kElfFile);

    EXPECT_EQ(no_symbols_module_info->name(), "no_symbols_elf");
    EXPECT_EQ(no_symbols_module_info->file_path(), no_symbols_path);
    EXPECT_EQ(no_symbols_module_info->file_size(), 18768);
    EXPECT_EQ(no_symbols_module_info->address_start(), 0x0);
    EXPECT_EQ(no_symbols_module_info->address_end(), 0x1000);
    EXPECT_EQ(no_symbols_module_info->build_id(), "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    EXPECT_EQ(no_symbols_module_info->load_bias(), 0x400000);
    EXPECT_EQ(no_symbols_module_info->object_file_type(), ModuleInfo::kElfFile);
  }
}

TEST(LinuxMap, ParseMapsWithSpacesInPath) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  // This file is a copy of hello_world_elf, but with the name containing spaces.
  const std::filesystem::path hello_world_path = test_path / "hello world elf";

  const std::string data{absl::StrFormat(
      "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     %s\n",
      hello_world_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  const ModuleInfo& hello_module_info = result.value()[0];
  EXPECT_EQ(hello_module_info.name(), "hello world elf");
  EXPECT_EQ(hello_module_info.file_path(), hello_world_path);
  EXPECT_EQ(hello_module_info.file_size(), 16616);
  EXPECT_EQ(hello_module_info.address_start(), 0x7f6874290000);
  EXPECT_EQ(hello_module_info.address_end(), 0x7f6874297000);
  EXPECT_EQ(hello_module_info.build_id(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
  EXPECT_EQ(hello_module_info.load_bias(), 0x0);
  EXPECT_EQ(hello_module_info.object_file_type(), ModuleInfo::kElfFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyAtExpectedOffset) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-103000 r-xp 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result.value()[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), 96441);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), 0x62640000);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyInMoreComplexExample) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  // The addresses in these maps are not page-aligned, but it doesn't matter for the test's purpose.
  const std::string data{
      absl::StrFormat("10000-11000 r--p 00000000 00:00 0    [stack]\n"
                      "100000-100C00 r--p 00000000 01:02 42    %s\n"  // The headers.
                      "100C00-100D00 rw-p 00000000 00:00 0\n"
                      "100D00-100E00 r--p 00000D00 01:02 42    %s\n"
                      "100E00-100F00 rw-p 00000000 00:00 0    [special]\n"
                      "100F00-101000 r--p 00000F00 01:02 42    %s\n"
                      "101000-103000 r-xp 00000000 00:00 0\n"  // The .text segment.
                      "200000-201000 r-xp 00000000 01:02 42    /path/to/nothing\n",
                      libtest_path, libtest_path, libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result.value()[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), 96441);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), 0x62640000);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedNotAnonymously) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-103000 r-xp 00001000 01:02 42    %s\n",
                      libtest_path, libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result.value()[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), 96441);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), 0x62640000);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyAtLowerThanExpectedOffset) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  // The addresses in these maps are not page-aligned, but it doesn't matter for the test's purpose.
  const std::string data{
      absl::StrFormat("100100-101000 r--p 00000100 01:02 42    %s\n"
                      "100F00-103000 r-xp 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result.value()[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), 96441);
  EXPECT_EQ(libtest_module_info.address_start(), 0x100F00);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), 0x62640000);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyAtExpectedOffsetAndFirstMapWithOffset) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  // The addresses in these maps are not page-aligned, but it doesn't matter for the test's purpose.
  const std::string data{
      absl::StrFormat("100100-101000 r--p 00000100 01:02 42    %s\n"
                      "101000-103000 r-xp 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result.value()[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), 96441);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), 0x62640000);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedWithWrongName) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-103000 r-xp 00000000 00:00 42    /wrong/path\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  EXPECT_EQ(result.value().size(), 0);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyButNotExecutable) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-103000 r--p 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  EXPECT_EQ(result.value().size(), 0);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyButExecutableMapAlreadyExists) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r-xp 00000000 01:02 42    %s\n"
                      "101000-103000 r-xp 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  ASSERT_EQ(result.value().size(), 1);

  // This comes from the first mapping, not the second.
  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result.value()[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), 96441);
  EXPECT_EQ(libtest_module_info.address_start(), 0x100000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x101000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), 0x62640000);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyAtOffsetTooHigh) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "102000-103000 r-xp 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  EXPECT_EQ(result.value().size(), 0);
}

TEST(LinuxMap, ParseMapsWithPeTextMappedAnonymouslyWithSizeTooSmall) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string data{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-102000 r-xp 00000000 00:00 0\n",
                      libtest_path)};
  const auto result = ParseMaps(data);
  ASSERT_THAT(result, HasNoError());
  EXPECT_EQ(result.value().size(), 0);
}

}  // namespace orbit_object_utils
