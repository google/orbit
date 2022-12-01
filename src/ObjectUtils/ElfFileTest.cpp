// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::SymbolInfo;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

namespace orbit_object_utils {

TEST(ElfFile, LoadDebugSymbols) {
  std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_debug_info";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  EXPECT_TRUE(elf_file->HasDebugSymbols());
  const auto symbols_result = elf_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 10);

  SymbolInfo& symbol_info = symbol_infos[0];
  EXPECT_EQ(symbol_info.demangled_name(), "deregister_tm_clones");
  EXPECT_EQ(symbol_info.address(), 0x1080);
  EXPECT_EQ(symbol_info.size(), 0);

  symbol_info = symbol_infos[5];
  EXPECT_EQ(symbol_info.demangled_name(), "main");
  EXPECT_EQ(symbol_info.address(), 0x1140);
  EXPECT_EQ(symbol_info.size(), 45);
}

TEST(ElfFile, HasDebugSymbols) {
  {
    const std::filesystem::path elf_with_symbols_path =
        orbit_test::GetTestdataDir() / "hello_world_elf";

    auto elf_with_symbols = CreateElfFile(elf_with_symbols_path);
    ASSERT_THAT(elf_with_symbols, HasNoError());

    EXPECT_TRUE(elf_with_symbols.value()->HasDebugSymbols());
  }
  {
    const std::filesystem::path elf_without_symbols_path =
        orbit_test::GetTestdataDir() / "no_symbols_elf";

    auto elf_without_symbols = CreateElfFile(elf_without_symbols_path);
    ASSERT_THAT(elf_without_symbols, HasNoError());

    EXPECT_FALSE(elf_without_symbols.value()->HasDebugSymbols());
  }
}

TEST(ElfFile, LoadSymbolsFromDynsymFails) {
  std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_debug_info";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  EXPECT_TRUE(elf_file->HasDynsym());
  const auto symbols_result = elf_file->LoadSymbolsFromDynsym();
  EXPECT_FALSE(symbols_result.has_value());
  EXPECT_THAT(symbols_result.error().message(),
              "Unable to load symbols from .dynsym section: not even a single symbol of type "
              "function found.");
}

TEST(ElfFile, LoadSymbolsFromDynsym) {
  // test_lib.so is copied from
  // build_clang9_relwithdebinfo/lib/libUserSpaceInstrumentationTestLib.so and stripped.
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "test_lib.so";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  EXPECT_TRUE(elf_file->HasDynsym());
  const auto symbols_result = elf_file->LoadSymbolsFromDynsym();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  ASSERT_EQ(symbol_infos.size(), 8);

  SymbolInfo& symbol_info = symbol_infos[7];
  EXPECT_EQ(symbol_info.demangled_name(), "UseTestLib");
  EXPECT_EQ(symbol_info.address(), 0x2670);
  EXPECT_EQ(symbol_info.size(), 591);
}

TEST(ElfFile, HasDynsym) {
  {
    const std::filesystem::path shared_object_path =
        orbit_test::GetTestdataDir() / "libtest-1.0.so";

    auto shared_object = CreateElfFile(shared_object_path);
    ASSERT_THAT(shared_object, HasNoError());

    EXPECT_TRUE(shared_object.value()->HasDynsym());
  }
  {
    const std::filesystem::path static_elf_path =
        orbit_test::GetTestdataDir() / "hello_world_static_elf";

    auto static_elf = CreateElfFile(static_elf_path);
    ASSERT_THAT(static_elf, HasNoError());

    EXPECT_FALSE(static_elf.value()->HasDynsym());
  }
}

static ::testing::Matcher<SymbolInfo> SymbolInfoEq(std::string_view demangled_name,
                                                   uint64_t address, uint64_t size,
                                                   bool is_hotpatchable) {
  return testing::AllOf(
      testing::Property("demangled_name", &SymbolInfo::demangled_name, demangled_name),
      testing::Property("address", &SymbolInfo::address, address),
      testing::Property("size", &SymbolInfo::size, size),
      testing::Property("is_hotpatchable", &SymbolInfo::is_hotpatchable, is_hotpatchable));
}

TEST(ElfFile, LoadPatchableFunctionEntryFromDebugSymbols) {
  const std::filesystem::path elf_path =
      orbit_test::GetTestdataDir() / "elf_binary_with_patchable_function_entries";
  auto elf_file_result = CreateElfFile(elf_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  const std::unique_ptr<ElfFile>& elf_file = elf_file_result.value();
  const auto symbols_result = elf_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  const std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                             symbols_result.value().symbol_infos().end());
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq("fun(int)", /*address=*/0x11D5,
                                                           /*size=*/17, /*is_hotpatchable=*/true)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq("main", /*address=*/0x11F5, /*size=*/76,
                                                           /*is_hotpatchable=*/true)));
}

TEST(ElfFile, LoadPatchableFunctionEntryFromEhOrDebugFrameEntries) {
  const std::filesystem::path elf_path =
      orbit_test::GetTestdataDir() / "elf_binary_with_patchable_function_entries";
  auto elf_file_result = CreateElfFile(elf_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  const std::unique_ptr<ElfFile>& elf_file = elf_file_result.value();
  const auto symbols_result = elf_file->LoadEhOrDebugFrameEntriesAsSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  const std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                             symbols_result.value().symbol_infos().end());
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq("[function@0x11d5]", /*address=*/0x11D5,
                                                           /*size=*/17, /*is_hotpatchable=*/true)));
  EXPECT_THAT(symbol_infos,
              testing::Contains(SymbolInfoEq("[function@0x11f5]", /*address=*/0x11F5, /*size=*/76,
                                             /*is_hotpatchable=*/true)));
}

TEST(ElfFile, LoadEhOrDebugFrameEntriesAsSymbolsFromEhFrame) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  const std::unique_ptr<ElfFile>& elf_file = elf_file_result.value();

  const auto symbols_result = elf_file->LoadEhOrDebugFrameEntriesAsSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  // These can be obtained with `objdump hello_world_elf --dwarf=frames` looking at the FDE
  // entries.
  EXPECT_THAT(symbol_infos, testing::ElementsAre(
                                SymbolInfoEq("[function@0x1050]", /*address=*/0x1050, /*size=*/43,
                                             /*is_hotpatchable=*/false),  // `_start`
                                SymbolInfoEq("[function@0x1020]", /*address=*/0x1020, /*size=*/32,
                                             /*is_hotpatchable=*/false),  // no function, `.plt`
                                SymbolInfoEq("[function@0x1040]", /*address=*/0x1040, /*size=*/8,
                                             /*is_hotpatchable=*/false),  // no function, `.plt.got`
                                SymbolInfoEq("[function@0x1135]", /*address=*/0x1135, /*size=*/35,
                                             /*is_hotpatchable=*/false),  // `main`
                                SymbolInfoEq("[function@0x1160]", /*address=*/0x1160, /*size=*/93,
                                             /*is_hotpatchable=*/false),  // `__libc_csu_init`
                                SymbolInfoEq("[function@0x11c0]", /*address=*/0x11c0, /*size=*/1,
                                             /*is_hotpatchable=*/false)));  // `__libc_csu_fini`
}

TEST(ElfFile, LoadEhOrDebugFrameEntriesAsSymbolsFromDebugFrame) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "debug_frame";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  const std::unique_ptr<ElfFile>& elf_file = elf_file_result.value();

  const auto symbols_result = elf_file->LoadEhOrDebugFrameEntriesAsSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  // There is only one function, the `main` function.
  EXPECT_THAT(symbol_infos,
              testing::ElementsAre(SymbolInfoEq("[function@0x1140]", /*address=*/0x1140,
                                                /*size=*/22, /*is_hotpatchable=*/false)));
}

TEST(ElfFile, LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbolsWithoutDynsym) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto elf_file = CreateObjectFile(file_path);
  ASSERT_THAT(elf_file, HasNoError());
  ASSERT_TRUE(elf_file.value()->IsElf());

  const auto fallback_symbols =
      elf_file.value()->LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols();
  ASSERT_THAT(fallback_symbols, HasNoError());

  std::vector<SymbolInfo> symbol_infos(fallback_symbols.value().symbol_infos().begin(),
                                       fallback_symbols.value().symbol_infos().end());
  EXPECT_THAT(symbol_infos, testing::ElementsAre(
                                SymbolInfoEq("[function@0x1050]", /*address=*/0x1050, /*size=*/43,
                                             /*is_hotpatchable=*/false),  // `_start`
                                SymbolInfoEq("[function@0x1020]", /*address=*/0x1020, /*size=*/32,
                                             /*is_hotpatchable=*/false),  // no function, `.plt`
                                SymbolInfoEq("[function@0x1040]", /*address=*/0x1040, /*size=*/8,
                                             /*is_hotpatchable=*/false),  // no function, `.plt.got`
                                SymbolInfoEq("[function@0x1135]", /*address=*/0x1135, /*size=*/35,
                                             /*is_hotpatchable=*/false),  // `main`
                                SymbolInfoEq("[function@0x1160]", /*address=*/0x1160, /*size=*/93,
                                             /*is_hotpatchable=*/false),  // `__libc_csu_init`
                                SymbolInfoEq("[function@0x11c0]", /*address=*/0x11c0, /*size=*/1,
                                             /*is_hotpatchable=*/false)));  // `__libc_csu_fini`
}

TEST(ElfFile, LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbolsWithDynsym) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest-1.0.so";

  auto elf_file = CreateObjectFile(file_path);
  ASSERT_THAT(elf_file, HasNoError());
  ASSERT_TRUE(elf_file.value()->IsElf());

  const auto fallback_symbols =
      elf_file.value()->LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols();
  ASSERT_THAT(fallback_symbols, HasNoError());

  std::vector<SymbolInfo> symbol_infos(fallback_symbols.value().symbol_infos().begin(),
                                       fallback_symbols.value().symbol_infos().end());
  EXPECT_THAT(
      symbol_infos,
      testing::ElementsAre(SymbolInfoEq("PrintHelloWorld", /*address=*/0x1110, /*size=*/12,
                                        /*is_hotpatchable=*/false),
                           SymbolInfoEq("[function@0x1020]", /*address=*/0x1020, /*size=*/32,
                                        /*is_hotpatchable=*/false),  // no function, `.plt`
                           SymbolInfoEq("[function@0x1040]", /*address=*/0x1040, /*size=*/8,
                                        /*is_hotpatchable=*/false)));  // no function, `.plt.got`
}

TEST(ElfFile, LoadBiasAndExecutableSegmentOffsetAndImageSize) {
  const std::filesystem::path test_elf_file_dynamic =
      orbit_test::GetTestdataDir() / "hello_world_elf";
  auto elf_file_dynamic = CreateElfFile(test_elf_file_dynamic);
  ASSERT_THAT(elf_file_dynamic, HasNoError());
  EXPECT_EQ(elf_file_dynamic.value()->GetLoadBias(), 0x0);
  EXPECT_EQ(elf_file_dynamic.value()->GetExecutableSegmentOffset(), 0x1000);
  EXPECT_EQ(elf_file_dynamic.value()->GetImageSize(), 0x4038);
}

TEST(ElfFile, LoadBiasAndExecutableSegmentOffsetAndImageSizeStatic) {
  const std::filesystem::path test_elf_file_static =
      orbit_test::GetTestdataDir() / "hello_world_static_elf";
  auto elf_file_static = CreateElfFile(test_elf_file_static);
  ASSERT_THAT(elf_file_static, HasNoError());
  EXPECT_EQ(elf_file_static.value()->GetLoadBias(), 0x400000);
  EXPECT_EQ(elf_file_static.value()->GetExecutableSegmentOffset(), 0x1000);
  EXPECT_EQ(elf_file_static.value()->GetImageSize(), 0xaaaa0);
}

TEST(ElfFile, ObjectSegments) {
  const std::filesystem::path test_elf_file_dynamic =
      orbit_test::GetTestdataDir() / "hello_world_elf";
  auto elf_file_dynamic = CreateElfFile(test_elf_file_dynamic);
  ASSERT_THAT(elf_file_dynamic, HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment> segments =
      elf_file_dynamic.value()->GetObjectSegments();
  ASSERT_EQ(segments.size(), 4);

  EXPECT_EQ(segments[0].offset_in_file(), 0);
  EXPECT_EQ(segments[0].size_in_file(), 0x568);
  EXPECT_EQ(segments[0].address(), 0);
  EXPECT_EQ(segments[0].size_in_memory(), 0x568);

  EXPECT_EQ(segments[1].offset_in_file(), 0x1000);
  EXPECT_EQ(segments[1].size_in_file(), 0x1cd);
  EXPECT_EQ(segments[1].address(), 0x1000);
  EXPECT_EQ(segments[1].size_in_memory(), 0x1cd);

  EXPECT_EQ(segments[2].offset_in_file(), 0x2000);
  EXPECT_EQ(segments[2].size_in_file(), 0x160);
  EXPECT_EQ(segments[2].address(), 0x2000);
  EXPECT_EQ(segments[2].size_in_memory(), 0x160);

  EXPECT_EQ(segments[3].offset_in_file(), 0x2de8);
  EXPECT_EQ(segments[3].size_in_file(), 0x248);
  EXPECT_EQ(segments[3].address(), 0x3de8);
  EXPECT_EQ(segments[3].size_in_memory(), 0x250);
}

TEST(ElfFile, ObjectSegmentsStatic) {
  const std::filesystem::path test_elf_file_static =
      orbit_test::GetTestdataDir() / "hello_world_static_elf";
  auto elf_file_static = CreateElfFile(test_elf_file_static);
  ASSERT_THAT(elf_file_static, HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment> segments =
      elf_file_static.value()->GetObjectSegments();
  ASSERT_EQ(segments.size(), 4);

  EXPECT_EQ(segments[0].offset_in_file(), 0);
  EXPECT_EQ(segments[0].size_in_file(), 0x4a8);
  EXPECT_EQ(segments[0].address(), 0x400000);
  EXPECT_EQ(segments[0].size_in_memory(), 0x4a8);

  EXPECT_EQ(segments[1].offset_in_file(), 0x1000);
  EXPECT_EQ(segments[1].size_in_file(), 0x7b4e1);
  EXPECT_EQ(segments[1].address(), 0x401000);
  EXPECT_EQ(segments[1].size_in_memory(), 0x7b4e1);

  EXPECT_EQ(segments[2].offset_in_file(), 0x7d000);
  EXPECT_EQ(segments[2].size_in_file(), 0x257f0);
  EXPECT_EQ(segments[2].address(), 0x47d000);
  EXPECT_EQ(segments[2].size_in_memory(), 0x257f0);

  EXPECT_EQ(segments[3].offset_in_file(), 0xa3060);
  EXPECT_EQ(segments[3].size_in_file(), 0x5270);
  EXPECT_EQ(segments[3].address(), 0x4a4060);
  EXPECT_EQ(segments[3].size_in_memory(), 0x6a40);
}

TEST(ElfFile, CalculateLoadBiasNoProgramHeaders) {
  const std::filesystem::path test_elf_file =
      orbit_test::GetTestdataDir() / "hello_world_elf_no_program_headers";
  auto elf_file_result = CreateElfFile(test_elf_file);

  ASSERT_THAT(
      elf_file_result,
      HasError(absl::StrFormat(
          "Unable to get load bias of ELF file: \"%s\". No executable PT_LOAD segment found.",
          test_elf_file.string())));
}

TEST(ElfFile, GetBuildId) {
  const std::filesystem::path hello_world_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto hello_world = CreateElfFile(hello_world_path);
  ASSERT_THAT(hello_world, HasNoError());
  EXPECT_EQ(hello_world.value()->GetBuildId(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");

  const std::filesystem::path elf_without_build_id_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_no_build_id";

  auto elf_without_build_id = CreateElfFile(elf_without_build_id_path);
  ASSERT_THAT(elf_without_build_id, HasNoError());
  EXPECT_EQ(elf_without_build_id.value()->GetBuildId(), "");
}

TEST(ElfFile, GetFilePath) {
  const std::filesystem::path hello_world_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto hello_world = CreateElfFile(hello_world_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_EQ(hello_world.value()->GetFilePath(), hello_world_path);
}

TEST(ElfFile, CreateFromBuffer) {
  const std::filesystem::path test_elf_file = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto file_content = orbit_base::ReadFileToString(test_elf_file);
  ASSERT_THAT(file_content, HasNoError());
  const std::string& buffer = file_content.value();
  ASSERT_NE(buffer.size(), 0);

  auto elf_file = CreateElfFileFromBuffer(test_elf_file, buffer.data(), buffer.size());
  ASSERT_THAT(elf_file, HasNoError());
  EXPECT_EQ(elf_file.value()->GetBuildId(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
}

TEST(ElfFile, FileDoesNotExist) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "does_not_exist";

  auto elf_file_or_error = CreateElfFile(file_path);
  ASSERT_TRUE(elf_file_or_error.has_error());
  EXPECT_THAT(absl::AsciiStrToLower(elf_file_or_error.error().message()),
              testing::HasSubstr("no such file or directory"));
}

TEST(ElfFile, HasDebugInfo) {
  const std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_debug_info";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_TRUE(hello_world.value()->HasDebugInfo());
}

TEST(ElfFile, DoesNotHaveDebugInfo) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_FALSE(hello_world.value()->HasDebugInfo());
}

static void RunLineInfoTest(const char* file_name) {
#ifdef _WIN32
  constexpr const char* const kSourcePath = "/ssd/local\\hello.cpp";
#else
  constexpr const char* const kSourcePath = "/ssd/local/hello.cpp";
#endif
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / file_name;

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  auto line_info1 = hello_world.value()->GetLineInfo(0x1140);
  ASSERT_THAT(line_info1, HasNoError());

  EXPECT_EQ(line_info1.value().source_file(), kSourcePath);
  EXPECT_EQ(line_info1.value().source_line(), 3);

  auto line_info2 = hello_world.value()->GetLineInfo(0x1150);
  ASSERT_THAT(line_info2, HasNoError());

  EXPECT_EQ(line_info2.value().source_file(), kSourcePath);
  EXPECT_EQ(line_info2.value().source_line(), 4);

  auto line_info_invalid_address = hello_world.value()->GetLineInfo(0x10);
  ASSERT_TRUE(line_info_invalid_address.has_error());
  EXPECT_THAT(line_info_invalid_address.error().message(),
              testing::HasSubstr("Unable to get line info for address=0x10"));
}

TEST(ElfFile, LineInfo) { RunLineInfoTest("hello_world_elf_with_debug_info"); }

TEST(ElfFile, LineInfoOnlyDebug) { RunLineInfoTest("hello_world_elf.debug"); }

TEST(ElfFile, LineInfoNoDebugInfo) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_FALSE(hello_world.value()->HasDebugInfo());

  EXPECT_DEATH((void)hello_world.value()->GetLineInfo(0x1140), "");
}

TEST(ElfFile, HasNoGnuDebugLink) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_FALSE(hello_world.value()->HasGnuDebuglink());
  EXPECT_FALSE(hello_world.value()->GetGnuDebugLinkInfo().has_value());
}

TEST(ElfFile, HasGnuDebugLink) {
  const std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_gnu_debuglink";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_TRUE(hello_world.value()->HasGnuDebuglink());
  EXPECT_TRUE(hello_world.value()->GetGnuDebugLinkInfo().has_value());
  EXPECT_EQ(hello_world.value()->GetGnuDebugLinkInfo()->path.string(), "hello_world_elf.debug");
}

TEST(ElfFile, CalculateDebuglinkChecksumValid) {
  const std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_gnu_debuglink";
  const std::filesystem::path debuglink_file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf.debug";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());
  ASSERT_TRUE(hello_world.value()->GetGnuDebugLinkInfo().has_value());

  const ErrorMessageOr<uint32_t> checksum_or_error =
      ElfFile::CalculateDebuglinkChecksum(debuglink_file_path);
  ASSERT_THAT(checksum_or_error, HasNoError());
  EXPECT_EQ(hello_world.value()->GetGnuDebugLinkInfo()->crc32_checksum, checksum_or_error.value());
}

TEST(ElfFile, CalculateDebuglinkChecksumNotFound) {
  const std::filesystem::path debuglink_file_path =
      orbit_test::GetTestdataDir() / "invalid_non_existing_filename.xyz";

  const ErrorMessageOr<uint32_t> checksum_or_error =
      ElfFile::CalculateDebuglinkChecksum(debuglink_file_path);
  EXPECT_TRUE(checksum_or_error.has_error());
}

TEST(ElfFile, LineInfoInlining) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "line_info_test_binary";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kFirstInstructionOfInlinedPrintHelloWorld = 0x401141;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> line_info =
      program.value()->GetLineInfo(kFirstInstructionOfInlinedPrintHelloWorld);
  ASSERT_THAT(line_info, HasNoError());

  EXPECT_EQ(line_info.value().source_line(), 13);
  EXPECT_EQ(std::filesystem::path{line_info.value().source_file()}.filename().string(),
            "LineInfoTestBinary.cpp");
}

TEST(ElfFile, CompressedDebugInfo) {
  const std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "line_info_test_binary_compressed";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kFirstInstructionOfInlinedPrintHelloWorld = 0x401141;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> line_info =
      program.value()->GetLineInfo(kFirstInstructionOfInlinedPrintHelloWorld);
  ASSERT_THAT(line_info, HasNoError());
}

TEST(ElfFile, GetSonameSmoke) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest-1.0.so";

  auto elf_file_or_error = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_or_error, HasNoError());

  EXPECT_EQ(elf_file_or_error.value()->GetSoname(), "libtest.so");
}

TEST(ElfFile, GetNameForFileWithoutSoname) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "hello_world_elf";

  auto elf_file_or_error = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_or_error, HasNoError());

  EXPECT_EQ(elf_file_or_error.value()->GetSoname(), "");
}

TEST(ElfFile, GetDeclarationLocationOfFunction) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "line_info_test_binary";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kAddressOfMainFunction = 0x401140;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> decl_line_info =
      program.value()->GetDeclarationLocationOfFunction(kAddressOfMainFunction);
  ASSERT_THAT(decl_line_info, HasNoError());

  EXPECT_EQ(decl_line_info.value().source_line(), 12);
  EXPECT_EQ(std::filesystem::path{decl_line_info.value().source_file()}.filename().string(),
            "LineInfoTestBinary.cpp");
}

TEST(ElfFile, GetDeclarationLocationOfFunctionLibc) {
// TODO(https://github.com/google/orbit/issues/4502): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libc.debug";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kAddressOfFunction = 0x20b20;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> decl_line_info =
      program.value()->GetDeclarationLocationOfFunction(kAddressOfFunction);
  ASSERT_THAT(decl_line_info, HasNoError());

  EXPECT_EQ(decl_line_info.value().source_line(), 31);
  EXPECT_EQ(std::filesystem::path{decl_line_info.value().source_file()}.filename().string(),
            "gconv_open.c");
}

TEST(ElfFile, GetLocationOfFunctionLibc) {
// TODO(https://github.com/google/orbit/issues/4502): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libc.debug";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kAddressOfFunction = 0x20b20;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> decl_line_info =
      program.value()->GetDeclarationLocationOfFunction(kAddressOfFunction);
  ASSERT_THAT(decl_line_info, HasNoError());

  EXPECT_EQ(decl_line_info.value().source_line(), 31);
  EXPECT_EQ(std::filesystem::path{decl_line_info.value().source_file()}.filename().string(),
            "gconv_open.c");
}

TEST(ElfFile, GetLocationOfFunctionNoSubroutine) {
// TODO(https://github.com/google/orbit/issues/4502): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libc.debug";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kAddressOfFunction = 0x10a0e0;
  EXPECT_THAT(program.value()->GetDeclarationLocationOfFunction(kAddressOfFunction),
              orbit_test_utils::HasError("Address not associated with any subroutine"));

  ErrorMessageOr<orbit_grpc_protos::LineInfo> function_location =
      program.value()->GetLocationOfFunction(kAddressOfFunction);
  ASSERT_THAT(function_location, HasNoError());

  EXPECT_EQ(function_location.value().source_line(), 90);
  EXPECT_EQ(std::filesystem::path{function_location.value().source_file()}.filename().string(),
            "auth_none.c");
}

}  // namespace orbit_object_utils