// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "ModuleUtils/ReadLinuxMaps.h"
#include "ModuleUtils/ReadLinuxModules.h"
#include "OrbitBase/File.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_test_utils::HasNoError;

namespace orbit_module_utils {

constexpr uint64_t kHelloWorldElfFileSize = 16616;
constexpr const char* kHelloWorldElfBuildId = "d12d54bc5b72ccce54a408bdeda65e2530740ac8";

static void VerifyObjectSegmentsForHelloWorldElf(
    const google::protobuf::RepeatedPtrField<ModuleInfo::ObjectSegment>& segments) {
  ASSERT_EQ(segments.size(), 4);
  // Simple sanity check, don't verify every single segment.
  EXPECT_EQ(segments[0].offset_in_file(), 0);
  EXPECT_EQ(segments[0].size_in_file(), 0x568);
  EXPECT_EQ(segments[0].address(), 0);
  EXPECT_EQ(segments[0].size_in_memory(), 0x568);
}

constexpr uint64_t kLibTestDllImageBase = 0x62640000;
constexpr uint64_t kLibTestDllFileSize = 96441;

static void VerifyObjectSegmentsForLibTestDll(
    const google::protobuf::RepeatedPtrField<ModuleInfo::ObjectSegment>& segments) {
  ASSERT_EQ(segments.size(), 19);
  EXPECT_EQ(segments[0].offset_in_file(), 0x600);
  EXPECT_EQ(segments[0].size_in_file(), 0x1400);
  EXPECT_EQ(segments[0].address(), kLibTestDllImageBase + 0x1000);
  EXPECT_EQ(segments[0].size_in_memory(), 0x1338);
}

TEST(ReadLinuxModules, CreateModuleElf) {
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
  EXPECT_EQ(result.value().build_id(), kHelloWorldElfBuildId);
  EXPECT_EQ(result.value().load_bias(), 0x0);
  EXPECT_EQ(result.value().object_file_type(), ModuleInfo::kElfFile);
  VerifyObjectSegmentsForHelloWorldElf(result.value().object_segments());
}

TEST(ReadLinuxModules, CreateModuleInDev) {
  const std::filesystem::path dev_zero_path = "/dev/zero";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(dev_zero_path, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_EQ(result.error().message(),
            "The module \"/dev/zero\" is a character or block device (is in /dev/)");
}

TEST(ReadLinuxModules, CreateModuleCoff) {
  const std::filesystem::path dll_path = orbit_test::GetTestdataDir() / "libtest.dll";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;

  auto result = CreateModule(dll_path, kStartAddress, kEndAddress);
  ASSERT_THAT(result, HasNoError());

  EXPECT_EQ(result.value().name(), "libtest.dll");
  EXPECT_EQ(result.value().file_path(), dll_path);
  EXPECT_EQ(result.value().file_size(), kLibTestDllFileSize);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().load_bias(), kLibTestDllImageBase);
  EXPECT_EQ(result.value().executable_segment_offset(), 0x1000);
  EXPECT_EQ(result.value().build_id(), "");
  EXPECT_EQ(result.value().object_file_type(), ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibTestDll(result.value().object_segments());
}

TEST(ReadLinuxModules, CreateModuleWithSoname) {
  const std::filesystem::path libtest_path = orbit_test::GetTestdataDir() / "libtest-1.0.so";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(libtest_path, kStartAddress, kEndAddress);
  ASSERT_THAT(result, HasNoError());

  EXPECT_EQ(result.value().name(), "libtest.so");
  EXPECT_EQ(result.value().file_path(), libtest_path);
  EXPECT_EQ(result.value().file_size(), 16128);
  EXPECT_EQ(result.value().address_start(), kStartAddress);
  EXPECT_EQ(result.value().address_end(), kEndAddress);
  EXPECT_EQ(result.value().build_id(), "2e70049c5cf42e6c5105825b57104af5882a40a2");
  EXPECT_EQ(result.value().load_bias(), 0x0);
  EXPECT_EQ(result.value().object_file_type(), ModuleInfo::kElfFile);
  ASSERT_EQ(result.value().object_segments().size(), 4);
  EXPECT_EQ(result.value().object_segments()[0].offset_in_file(), 0);
  EXPECT_EQ(result.value().object_segments()[0].size_in_file(), 0x4e0);
  EXPECT_EQ(result.value().object_segments()[0].address(), 0);
  EXPECT_EQ(result.value().object_segments()[0].size_in_memory(), 0x4e0);
}

TEST(ReadLinuxModules, CreateModuleNotAnObject) {
  const std::filesystem::path text_file = orbit_test::GetTestdataDir() / "textfile.txt";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(text_file, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_THAT(result.error().message(),
              testing::HasSubstr("The file was not recognized as a valid object file"));
}

TEST(ReadLinuxModules, CreateModuleFileDoesNotExist) {
  const std::filesystem::path file_path = "/not/a/valid/file/path";

  constexpr uint64_t kStartAddress = 23;
  constexpr uint64_t kEndAddress = 8004;
  auto result = CreateModule(file_path, kStartAddress, kEndAddress);
  ASSERT_TRUE(result.has_error());
  EXPECT_EQ(result.error().message(), "The module file \"/not/a/valid/file/path\" does not exist");
}

TEST(ReadLinuxModules, ReadModules) {
  const auto result = ReadModules(getpid());
  EXPECT_THAT(result, HasNoError());
  EXPECT_GT(result.value().size(), 0);
}

TEST(ReadLinuxModules, ReadModulesFromMapsEmptyData) {
  const auto result = ReadModulesFromMaps(ParseMaps(""));
  EXPECT_EQ(result.size(), 0);
}

TEST(ReadLinuxModules, ReadModulesFromMaps1) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path hello_world_path = test_path / "hello_world_elf";
  const std::filesystem::path text_file = test_path / "textfile.txt";

  // Only testing the correct size of the result. The entry with /dev/zero is ignored due to the
  // path starting with /dev/. The last entry has a valid path, but the executable flag is not set.
  const std::string proc_pid_maps_content{absl::StrFormat(
      "7f687428f000-7f6874290000 r-xp 00009000 fe:01 661216                     /path/to/nothing\n"
      "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     %s\n"
      "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     /dev/zero\n"
      "7f6874290001-7f6874297002 r-dp 00000000 fe:01 661214                     %s\n",
      hello_world_path, text_file)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  EXPECT_EQ(result.size(), 1);
}

TEST(ReadLinuxModules, ReadModulesFromMaps2) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path hello_world_path = test_path / "hello_world_elf";
  const std::filesystem::path no_symbols_path = test_path / "no_symbols_elf";

  const std::string proc_pid_maps_content{absl::StrFormat(
      "7f6874285000-7f6874288000 r--p 00000000 fe:01 661216                     %1$s\n"
      "7f6874288000-7f687428c000 r-xp 00003000 fe:01 661216                     %1$s\n"
      "7f687428c000-7f687428e000 r--p 00007000 fe:01 661216                     %1$s\n"
      "7f687428e000-7f687428f000 r--p 00008000 fe:01 661216                     %1$s\n"
      "7f687428f000-7f6874290000 rw-p 00009000 fe:01 661216                     %1$s\n"
      "800000000000-800000001000 r-xp 00009000 fe:01 661216                     %2$s\n",
      hello_world_path, no_symbols_path)};

  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 2);

  const ModuleInfo& hello_module_info = result[0];
  const ModuleInfo& no_symbols_module_info = result[1];

  EXPECT_EQ(hello_module_info.name(), "hello_world_elf");
  EXPECT_EQ(hello_module_info.file_path(), hello_world_path);
  EXPECT_EQ(hello_module_info.file_size(), kHelloWorldElfFileSize);
  EXPECT_EQ(hello_module_info.address_start(), 0x7f6874288000);
  EXPECT_EQ(hello_module_info.address_end(), 0x7f687428c000);
  EXPECT_EQ(hello_module_info.build_id(), kHelloWorldElfBuildId);
  EXPECT_EQ(hello_module_info.load_bias(), 0x0);
  EXPECT_EQ(hello_module_info.object_file_type(), ModuleInfo::kElfFile);
  VerifyObjectSegmentsForHelloWorldElf(hello_module_info.object_segments());

  EXPECT_EQ(no_symbols_module_info.name(), "no_symbols_elf");
  EXPECT_EQ(no_symbols_module_info.file_path(), no_symbols_path);
  EXPECT_EQ(no_symbols_module_info.file_size(), 18768);
  EXPECT_EQ(no_symbols_module_info.address_start(), 0x800000000000);
  EXPECT_EQ(no_symbols_module_info.address_end(), 0x800000001000);
  EXPECT_EQ(no_symbols_module_info.build_id(), "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  EXPECT_EQ(no_symbols_module_info.load_bias(), 0x400000);
  EXPECT_EQ(no_symbols_module_info.object_file_type(), ModuleInfo::kElfFile);
  ASSERT_EQ(no_symbols_module_info.object_segments().size(), 4);
  EXPECT_EQ(no_symbols_module_info.object_segments()[0].offset_in_file(), 0);
  EXPECT_EQ(no_symbols_module_info.object_segments()[0].size_in_file(), 0xa40);
  EXPECT_EQ(no_symbols_module_info.object_segments()[0].address(), 0x400000);
  EXPECT_EQ(no_symbols_module_info.object_segments()[0].size_in_memory(), 0xa40);
}

TEST(ReadLinuxModules, ReadModulesFromMapsWithSpacesInPath) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  ErrorMessageOr<std::string> elf_contents_or_error =
      orbit_base::ReadFileToString(test_path / "hello_world_elf");
  ASSERT_THAT(elf_contents_or_error, HasNoError());
  std::string& elf_contents = elf_contents_or_error.value();

  // This file is created as a copy of hello_world_elf, but with the name containing spaces.
  auto hello_world_elf_temporary_or_error =
      orbit_test_utils::TemporaryFile::Create("hello world elf");
  ASSERT_THAT(hello_world_elf_temporary_or_error, HasNoError());
  orbit_test_utils::TemporaryFile& hello_world_elf_temporary =
      hello_world_elf_temporary_or_error.value();

  ASSERT_THAT(orbit_base::WriteFully(hello_world_elf_temporary.fd(), elf_contents), HasNoError());

  const std::string proc_pid_maps_content{absl::StrFormat(
      "7f6874290000-7f6874297000 r-xp 00000000 fe:01 661214                     %s\n",
      hello_world_elf_temporary.file_path())};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const ModuleInfo& hello_module_info = result[0];
  EXPECT_EQ(hello_module_info.name(), hello_world_elf_temporary.file_path().filename().string());
  EXPECT_EQ(hello_module_info.file_path(), hello_world_elf_temporary.file_path());
  EXPECT_EQ(hello_module_info.file_size(), kHelloWorldElfFileSize);
  EXPECT_EQ(hello_module_info.address_start(), 0x7f6874290000);
  EXPECT_EQ(hello_module_info.address_end(), 0x7f6874297000);
  EXPECT_EQ(hello_module_info.build_id(), kHelloWorldElfBuildId);
  EXPECT_EQ(hello_module_info.load_bias(), 0x0);
  EXPECT_EQ(hello_module_info.object_file_type(), ModuleInfo::kElfFile);
  VerifyObjectSegmentsForHelloWorldElf(hello_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsElfWithMultipleExecutableMaps) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path hello_world_path = test_path / "hello_world_elf";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %1$s\n"
                      "101000-102000 r-xp 00000000 01:02 42    %1$s\n"
                      "102000-103000 r--p 00000000 01:02 42    %1$s\n"
                      "103000-104000 rw-p 00000000 00:00 0 \n"
                      "104000-105000 r-xp 00000000 01:02 42    %1$s\n",
                      hello_world_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const ModuleInfo& hello_module_info = result[0];
  EXPECT_EQ(hello_module_info.name(), "hello_world_elf");
  EXPECT_EQ(hello_module_info.file_path(), hello_world_path);
  EXPECT_EQ(hello_module_info.file_size(), kHelloWorldElfFileSize);
  EXPECT_EQ(hello_module_info.address_start(), 0x101000);
  EXPECT_EQ(hello_module_info.address_end(), 0x105000);
  EXPECT_EQ(hello_module_info.build_id(), kHelloWorldElfBuildId);
  EXPECT_EQ(hello_module_info.load_bias(), 0x0);
  EXPECT_EQ(hello_module_info.object_file_type(), ModuleInfo::kElfFile);
  VerifyObjectSegmentsForHelloWorldElf(hello_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedNotAnonymously) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";  // SizeOfImage = 0x20000

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %1$s\n"
                      "101000-103000 r-xp 00001000 01:02 42    %1$s\n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), kLibTestDllFileSize);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), kLibTestDllImageBase);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibTestDll(libtest_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedNotAnonymouslyWithMultipleExecutableMaps) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %1$s\n"
                      "101000-102000 r-xp 00000000 01:02 42    %1$s\n"
                      "102000-103000 r--p 00000000 01:02 42    %1$s\n"
                      "103000-104000 rw-p 00000000 00:00 0 \n"
                      "104000-105000 r-xp 00000000 01:02 42    %1$s\n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), kLibTestDllFileSize);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x105000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), kLibTestDllImageBase);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibTestDll(libtest_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedAnonymously) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %1$s\n"
                      "101000-103000 r-xp 00000000 00:00 0 \n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), kLibTestDllFileSize);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x103000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), kLibTestDllImageBase);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibTestDll(libtest_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedAnonymouslyWithMultipleExecutableMaps) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %1$s\n"
                      "101000-102000 r-xp 00000000 00:00 0 \n"
                      "102000-103000 r--p 00000000 00:00 0 \n"
                      "103000-104000 rw-p 00000000 00:00 0 \n"
                      "104000-105000 r-xp 00000000 00:00 0 \n"
                      "105000-121000 r-xp 00000000 00:00 0 \n",  // Beyond SizeOfImage.
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), kLibTestDllFileSize);
  EXPECT_EQ(libtest_module_info.address_start(), 0x101000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x105000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), kLibTestDllImageBase);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibTestDll(libtest_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedAnonymouslyInMoreComplexExample) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("10000-11000 r--p 00000000 00:00 0    [stack]\n"
                      "100000-101000 r--p 00000000 01:02 42    %1$s\n"  // The headers.
                      "101000-102000 rw-p 00000000 00:00 0 \n"
                      "102000-103000 r--p 00002000 01:02 42    %1$s\n"
                      "103000-104000 r-xp 00000000 00:00 0    [special]\n"
                      "104000-105000 r--p 00004000 01:02 42    %1$s\n"
                      "105000-106000 r-xp 00000000 00:00 0 \n"  // An executable map.
                      "106000-107000 r--p 00006000 01:02 42    %1$s\n"
                      "107000-108000 rw-p 00000000 00:00 0    [special]\n"
                      "108000-109000 r-xp 00000000 00:00 0 \n"  // An executable map.
                      "109000-10A000 r-xp 00000000 01:02 42    /path/to/nothing\n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 1);

  const orbit_grpc_protos::ModuleInfo& libtest_module_info = result[0];
  EXPECT_EQ(libtest_module_info.name(), "libtest.dll");
  EXPECT_EQ(libtest_module_info.file_path(), libtest_path);
  EXPECT_EQ(libtest_module_info.file_size(), kLibTestDllFileSize);
  EXPECT_EQ(libtest_module_info.address_start(), 0x105000);
  EXPECT_EQ(libtest_module_info.address_end(), 0x109000);
  EXPECT_EQ(libtest_module_info.build_id(), "");
  EXPECT_EQ(libtest_module_info.load_bias(), kLibTestDllImageBase);
  EXPECT_EQ(libtest_module_info.executable_segment_offset(), 0x1000);
  EXPECT_EQ(libtest_module_info.soname(), "");
  EXPECT_EQ(libtest_module_info.object_file_type(), orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibTestDll(libtest_module_info.object_segments());
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedAnonymouslyAndFirstMapWithOffset) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("101000-102000 r--p 00001000 01:02 42    %s\n"
                      "102000-103000 r-xp 00000000 00:00 0 \n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  ASSERT_EQ(result.size(), 0);
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedWithWrongName) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-103000 r-xp 00000000 00:00 42    /wrong/path\n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  EXPECT_EQ(result.size(), 0);
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeNoExecutableMap) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-103000 r--p 00000000 00:00 0 \n",
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  EXPECT_EQ(result.size(), 0);
}

TEST(ReadLinuxModules, ReadModulesFromMapsPeTextMappedAnonymouslyWithEndBeyondSizeOfImage) {
  const std::filesystem::path test_path = orbit_test::GetTestdataDir();
  const std::filesystem::path libtest_path = test_path / "libtest.dll";

  const std::string proc_pid_maps_content{
      absl::StrFormat("100000-101000 r--p 00000000 01:02 42    %s\n"
                      "101000-121000 r-xp 00000000 00:00 0 \n",  // Beyond SizeOfImage.
                      libtest_path)};
  const auto result = ReadModulesFromMaps(ParseMaps(proc_pid_maps_content));
  EXPECT_EQ(result.size(), 0);
}

}  // namespace orbit_module_utils
