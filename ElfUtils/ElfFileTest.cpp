// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <fstream>
#include <utility>

#include "ElfUtils/ElfFile.h"
#include "Path.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"
#include "symbol.pb.h"

using ElfUtils::ElfFile;
using orbit_grpc_protos::SymbolInfo;

TEST(ElfFile, LoadSymbols) {
  std::string executable_dir = Path::GetExecutableDir();
  std::string executable = "hello_world_elf";
  std::string file_path = executable_dir + "testdata/" + executable;

  auto elf_file_result = ElfFile::Create(file_path);
  ASSERT_TRUE(elf_file_result) << elf_file_result.error().message();
  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  const auto symbols_result = elf_file->LoadSymbols();
  ASSERT_TRUE(symbols_result);

  EXPECT_EQ(symbols_result.value().symbols_file_path(), file_path);

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 10);

  SymbolInfo& symbol_info = symbol_infos[0];
  EXPECT_EQ(symbol_info.name(), "deregister_tm_clones");
  EXPECT_EQ(symbol_info.demangled_name(), "deregister_tm_clones");
  EXPECT_EQ(symbol_info.address(), 0x1080);
  EXPECT_EQ(symbol_info.size(), 0);
  // TODO (b/154580143) have correct source file and line here
  EXPECT_EQ(symbol_info.source_file(), "");
  EXPECT_EQ(symbol_info.source_line(), 0);

  symbol_info = symbol_infos[9];
  EXPECT_EQ(symbol_info.name(), "main");
  EXPECT_EQ(symbol_info.demangled_name(), "main");
  EXPECT_EQ(symbol_info.address(), 0x1135);
  EXPECT_EQ(symbol_info.size(), 35);
  // TODO (b/154580143) have correct source file and line here
  EXPECT_EQ(symbol_info.source_file(), "");
  EXPECT_EQ(symbol_info.source_line(), 0);
}

TEST(ElfFile, CalculateLoadBias) {
  const std::string executable_dir = Path::GetExecutableDir();

  {
    const std::string test_elf_file_dynamic = executable_dir + "/testdata/hello_world_elf";
    auto elf_file_dynamic = ElfFile::Create(test_elf_file_dynamic);
    ASSERT_TRUE(elf_file_dynamic);
    const auto load_bias = elf_file_dynamic.value()->GetLoadBias();
    ASSERT_TRUE(load_bias) << load_bias.error().message();
    EXPECT_EQ(load_bias.value(), 0x0);
  }

  {
    const std::string test_elf_file_static = executable_dir + "/testdata/hello_world_static_elf";
    auto elf_file_static = ElfFile::Create(test_elf_file_static);
    ASSERT_TRUE(elf_file_static) << elf_file_static.error().message();
    const auto load_bias = elf_file_static.value()->GetLoadBias();
    ASSERT_TRUE(load_bias) << load_bias.error().message();
    EXPECT_EQ(load_bias.value(), 0x400000);
  }
}

TEST(ElfFile, CalculateLoadBiasNoProgramHeaders) {
  const std::string executable_dir = Path::GetExecutableDir();
  const std::string test_elf_file = executable_dir + "/testdata/hello_world_elf_no_program_headers";
  auto elf_file_result = ElfFile::Create(test_elf_file);

  ASSERT_TRUE(elf_file_result) << elf_file_result.error().message();
  auto elf_file = std::move(elf_file_result.value());
  const auto load_bias_result = elf_file->GetLoadBias();
  ASSERT_FALSE(load_bias_result);
  EXPECT_EQ(load_bias_result.error().message(),
            absl::StrFormat("Unable to get load bias of ELF file: \"%s\". No PT_LOAD program "
                            "headers found.",
                            test_elf_file));
}

TEST(ElfFile, HasSymtab) {
  std::string executable_dir = Path::GetExecutableDir();
  std::string elf_with_symbols_path = executable_dir + "/testdata/hello_world_elf";
  std::string elf_without_symbols_path = executable_dir + "/testdata/no_symbols_elf";

  auto elf_with_symbols = ElfFile::Create(elf_with_symbols_path);
  ASSERT_TRUE(elf_with_symbols) << elf_with_symbols.error().message();

  EXPECT_TRUE(elf_with_symbols.value()->HasSymtab());

  auto elf_without_symbols = ElfFile::Create(elf_without_symbols_path);
  ASSERT_TRUE(elf_without_symbols) << elf_without_symbols.error().message();

  EXPECT_FALSE(elf_without_symbols.value()->HasSymtab());
}

TEST(ElfFile, GetBuildId) {
  std::string executable_dir = Path::GetExecutableDir();
  std::string hello_world_path = executable_dir + "/testdata/hello_world_elf";

  auto hello_world = ElfFile::Create(hello_world_path);
  ASSERT_TRUE(hello_world) << hello_world.error().message();
  EXPECT_EQ(hello_world.value()->GetBuildId(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");

  std::string elf_without_build_id_path = executable_dir + "/testdata/hello_world_elf_no_build_id";

  auto elf_without_build_id = ElfFile::Create(elf_without_build_id_path);
  ASSERT_TRUE(elf_without_build_id) << elf_without_build_id.error().message();
  EXPECT_EQ(elf_without_build_id.value()->GetBuildId(), "");
}

TEST(ElfFile, GetFilePath) {
  std::string executable_dir = Path::GetExecutableDir();
  std::string hello_world_path = executable_dir + "/testdata/hello_world_elf";

  auto hello_world = ElfFile::Create(hello_world_path);
  ASSERT_TRUE(hello_world) << hello_world.error().message();

  EXPECT_EQ(hello_world.value()->GetFilePath(), hello_world_path);
}

TEST(ElfFile, CreateFromBuffer) {
  std::string executable_dir = Path::GetExecutableDir();
  std::string test_elf_file = executable_dir + "/testdata/hello_world_elf";

  std::ifstream test_elf_stream{test_elf_file, std::ios::binary};
  const std::string buffer{std::istreambuf_iterator<char>{test_elf_stream},
                           std::istreambuf_iterator<char>{}};
  ASSERT_NE(buffer.size(), 0);

  auto elf_file = ElfFile::CreateFromBuffer(test_elf_file, buffer.data(), buffer.size());
  ASSERT_TRUE(elf_file) << elf_file.error().message();
  EXPECT_EQ(elf_file.value()->GetBuildId(), "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
}

TEST(ElfFile, FileDoesNotExist) {
  std::string executable_dir = Path::GetExecutableDir();
  std::string file_path = executable_dir + "/testdata/does_not_exist";

  auto elf_file = ElfFile::Create(file_path);
  ASSERT_FALSE(elf_file);
  EXPECT_THAT(absl::AsciiStrToLower(elf_file.error().message()),
              testing::HasSubstr("no such file or directory"));
}
