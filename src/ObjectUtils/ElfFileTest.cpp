// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <iterator>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <utility>
#include <vector>

#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/TestUtils.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"
#include "symbol.pb.h"

using orbit_base::HasNoError;
using orbit_grpc_protos::SymbolInfo;
using orbit_object_utils::CreateElfFile;
using orbit_object_utils::CreateElfFileFromBuffer;
using orbit_object_utils::ElfFile;

TEST(ElfFile, LoadDebugSymbols) {
  std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf_with_debug_info";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  const auto symbols_result = elf_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  EXPECT_EQ(symbols_result.value().symbols_file_path(), file_path);

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 10);

  SymbolInfo& symbol_info = symbol_infos[0];
  EXPECT_EQ(symbol_info.name(), "deregister_tm_clones");
  EXPECT_EQ(symbol_info.demangled_name(), "deregister_tm_clones");
  EXPECT_EQ(symbol_info.address(), 0x1080);
  EXPECT_EQ(symbol_info.size(), 0);

  symbol_info = symbol_infos[5];
  EXPECT_EQ(symbol_info.name(), "main");
  EXPECT_EQ(symbol_info.demangled_name(), "main");
  EXPECT_EQ(symbol_info.address(), 0x1140);
  EXPECT_EQ(symbol_info.size(), 45);
}

TEST(ElfFile, LoadSymbolsFromDynsymFails) {
  std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf_with_debug_info";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  const auto symbols_result = elf_file->LoadSymbolsFromDynsym();
  EXPECT_FALSE(symbols_result.has_value());
  EXPECT_THAT(symbols_result.error().message(),
              "Unable to load symbols from .dynsym section, not even a single symbol of type "
              "function found.");
}

TEST(ElfFile, LoadSymbolsFromDynsym) {
  // test_lib.so is copied from
  // build_clang9_relwithdebinfo/lib/libUserSpaceInstrumentationTestLib.so and stripped.
  std::filesystem::path file_path = orbit_base::GetExecutableDir() / "testdata" / "test_lib.so";

  auto elf_file_result = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_result, HasNoError());
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  const auto symbols_result = elf_file->LoadSymbolsFromDynsym();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 8);

  SymbolInfo& symbol_info = symbol_infos[7];
  EXPECT_EQ(symbol_info.name(), "UseTestLib");
  EXPECT_EQ(symbol_info.demangled_name(), "UseTestLib");
  EXPECT_EQ(symbol_info.address(), 0x2670);
  EXPECT_EQ(symbol_info.size(), 591);
}

TEST(ElfFile, CalculateLoadBias) {
  std::filesystem::path executable_dir = orbit_base::GetExecutableDir();

  {
    const std::filesystem::path test_elf_file_dynamic =
        executable_dir / "testdata" / "hello_world_elf";
    auto elf_file_dynamic = CreateElfFile(test_elf_file_dynamic);
    ASSERT_THAT(elf_file_dynamic, HasNoError());
    const auto load_bias = elf_file_dynamic.value()->GetLoadBias();
    ASSERT_THAT(load_bias, HasNoError());
    EXPECT_EQ(load_bias.value(), 0x0);
  }

  {
    const std::filesystem::path test_elf_file_static =
        executable_dir / "testdata" / "hello_world_static_elf";
    auto elf_file_static = CreateElfFile(test_elf_file_static);
    ASSERT_THAT(elf_file_static, HasNoError());
    const auto load_bias = elf_file_static.value()->GetLoadBias();
    ASSERT_THAT(load_bias, HasNoError());
    EXPECT_EQ(load_bias.value(), 0x400000);
  }
}

TEST(ElfFile, CalculateLoadBiasNoProgramHeaders) {
  const std::filesystem::path test_elf_file =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf_no_program_headers";
  auto elf_file_result = CreateElfFile(test_elf_file);

  ASSERT_THAT(elf_file_result, HasNoError());
  auto elf_file = std::move(elf_file_result.value());
  const auto load_bias_or_error = elf_file->GetLoadBias();
  ASSERT_TRUE(load_bias_or_error.has_error());
  EXPECT_EQ(load_bias_or_error.error().message(),
            absl::StrFormat(
                "Unable to get load bias of ELF file: \"%s\". No executable PT_LOAD segment found.",
                test_elf_file.string()));
}

TEST(ElfFile, HasDebugSymbols) {
  const std::filesystem::path executable_dir = orbit_base::GetExecutableDir();
  const std::filesystem::path elf_with_symbols_path =
      executable_dir / "testdata" / "hello_world_elf";
  const std::filesystem::path elf_without_symbols_path =
      executable_dir / "testdata" / "no_symbols_elf";

  auto elf_with_symbols = CreateElfFile(elf_with_symbols_path);
  ASSERT_THAT(elf_with_symbols, HasNoError());

  EXPECT_TRUE(elf_with_symbols.value()->HasDebugSymbols());

  auto elf_without_symbols = CreateElfFile(elf_without_symbols_path);
  ASSERT_THAT(elf_without_symbols, HasNoError());

  EXPECT_FALSE(elf_without_symbols.value()->HasDebugSymbols());
}

TEST(ElfFile, GetBuildId) {
  const std::filesystem::path executable_dir = orbit_base::GetExecutableDir();
  const std::filesystem::path hello_world_path = executable_dir / "testdata" / "hello_world_elf";

  auto hello_world = CreateElfFile(hello_world_path);
  ASSERT_THAT(hello_world, HasNoError());
  EXPECT_EQ(hello_world.value()->GetBuildId(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");

  const std::filesystem::path elf_without_build_id_path =
      executable_dir / "testdata" / "hello_world_elf_no_build_id";

  auto elf_without_build_id = CreateElfFile(elf_without_build_id_path);
  ASSERT_THAT(elf_without_build_id, HasNoError());
  EXPECT_EQ(elf_without_build_id.value()->GetBuildId(), "");
}

TEST(ElfFile, GetFilePath) {
  const std::filesystem::path hello_world_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

  auto hello_world = CreateElfFile(hello_world_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_EQ(hello_world.value()->GetFilePath(), hello_world_path);
}

TEST(ElfFile, CreateFromBuffer) {
  const std::filesystem::path test_elf_file =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

  auto file_content = orbit_base::ReadFileToString(test_elf_file);
  ASSERT_THAT(file_content, HasNoError());
  const std::string& buffer = file_content.value();
  ASSERT_NE(buffer.size(), 0);

  auto elf_file = CreateElfFileFromBuffer(test_elf_file, buffer.data(), buffer.size());
  ASSERT_THAT(elf_file, HasNoError());
  EXPECT_EQ(elf_file.value()->GetBuildId(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
}

TEST(ElfFile, FileDoesNotExist) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "does_not_exist";

  auto elf_file_or_error = CreateElfFile(file_path);
  ASSERT_TRUE(elf_file_or_error.has_error());
  EXPECT_THAT(absl::AsciiStrToLower(elf_file_or_error.error().message()),
              testing::HasSubstr("no such file or directory"));
}

TEST(ElfFile, HasDebugInfo) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf_with_debug_info";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_TRUE(hello_world.value()->HasDebugInfo());
}

TEST(ElfFile, DoesNotHaveDebugInfo) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

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
  const std::filesystem::path file_path = orbit_base::GetExecutableDir() / "testdata" / file_name;

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
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_FALSE(hello_world.value()->HasDebugInfo());

  EXPECT_DEATH((void)hello_world.value()->GetLineInfo(0x1140), "");
}

TEST(ElfFile, HasNoGnuDebugLink) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_FALSE(hello_world.value()->HasGnuDebuglink());
  EXPECT_FALSE(hello_world.value()->GetGnuDebugLinkInfo().has_value());
}

TEST(ElfFile, HasGnuDebugLink) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf_with_gnu_debuglink";

  auto hello_world = CreateElfFile(file_path);
  ASSERT_THAT(hello_world, HasNoError());

  EXPECT_TRUE(hello_world.value()->HasGnuDebuglink());
  EXPECT_TRUE(hello_world.value()->GetGnuDebugLinkInfo().has_value());
  EXPECT_EQ(hello_world.value()->GetGnuDebugLinkInfo()->path.string(), "hello_world_elf.debug");
}

TEST(ElfFile, CalculateDebuglinkChecksumValid) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf_with_gnu_debuglink";
  const std::filesystem::path debuglink_file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf.debug";

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
      orbit_base::GetExecutableDir() / "testdata" / "invalid_non_existing_filename.xyz";

  const ErrorMessageOr<uint32_t> checksum_or_error =
      ElfFile::CalculateDebuglinkChecksum(debuglink_file_path);
  EXPECT_TRUE(checksum_or_error.has_error());
}

TEST(ElfFile, LineInfoInlining) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "line_info_test_binary";

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
      orbit_base::GetExecutableDir() / "testdata" / "line_info_test_binary_compressed";

  auto program = CreateElfFile(file_path);
  ASSERT_THAT(program, HasNoError());

  constexpr uint64_t kFirstInstructionOfInlinedPrintHelloWorld = 0x401141;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> line_info =
      program.value()->GetLineInfo(kFirstInstructionOfInlinedPrintHelloWorld);
  ASSERT_THAT(line_info, HasNoError());
}

TEST(ElfFile, GetSonameSmoke) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "libtest-1.0.so";

  auto elf_file_or_error = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_or_error, HasNoError());

  EXPECT_EQ(elf_file_or_error.value()->GetSoname(), "libtest.so");
}

TEST(ElfFile, GetNameForFileWithoutSoname) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "hello_world_elf";

  auto elf_file_or_error = CreateElfFile(file_path);
  ASSERT_THAT(elf_file_or_error, HasNoError());

  EXPECT_EQ(elf_file_or_error.value()->GetSoname(), "");
}

TEST(ElfFile, GetDeclarationLocationOfFunction) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "line_info_test_binary";

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