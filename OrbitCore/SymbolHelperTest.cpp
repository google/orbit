// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>

#include "Path.h"
#include "SymbolHelper.h"
#include "absl/strings/ascii.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleSymbols;

const std::string executable_directory = Path::GetExecutablePath() + "testdata/";

TEST(SymbolHelper, LoadSymbolsCollectorSameFile) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  SymbolHelper symbol_helper({executable_directory}, {});
  const auto debug_symbol_file_path =
      symbol_helper.FindDebugSymbolsFile(file_path, "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
  ASSERT_TRUE(debug_symbol_file_path) << debug_symbol_file_path.error().message();
  EXPECT_EQ(debug_symbol_file_path.value(), file_path);
  const auto symbols_result = symbol_helper.LoadSymbolsCollector(file_path);
  ASSERT_TRUE(symbols_result) << symbols_result.error().message();
  ModuleSymbols symbols = std::move(symbols_result.value());
  EXPECT_EQ(symbols.symbols_file_path(), file_path);
}

TEST(SymbolHelper, FindDebugInfoFileSameFile) {
  const std::string kFilePath = Path::JoinPath({executable_directory, "hello_world_elf"});
  const std::string kBuildId = "d12d54bc5b72ccce54a408bdeda65e2530740ac8";
  const std::string kInvalidBuildId = "invalid_build_id";
  const std::string kEmptyBuildId = "";

  SymbolHelper symbol_helper({executable_directory}, {});
  const auto debug_symbol_file_path = symbol_helper.FindDebugSymbolsFile(kFilePath, kBuildId);
  ASSERT_TRUE(debug_symbol_file_path) << debug_symbol_file_path.error().message();
  EXPECT_EQ(debug_symbol_file_path.value(), kFilePath);

  const auto invalid_build_id_result =
      symbol_helper.FindDebugSymbolsFile(kFilePath, kInvalidBuildId);
  ASSERT_FALSE(invalid_build_id_result);
  EXPECT_THAT(invalid_build_id_result.error().message(),
              testing::HasSubstr("Could not find a file with debug symbols"));

  const auto empty_build_id_result = symbol_helper.FindDebugSymbolsFile(kFilePath, kEmptyBuildId);
  ASSERT_TRUE(empty_build_id_result) << empty_build_id_result.error().message();
  EXPECT_EQ(empty_build_id_result.value(), kFilePath);
}

TEST(SymbolHelper, FindDebugSymbolsSeparateFile) {
  const std::string kFilePath = Path::JoinPath({executable_directory, "no_symbols_elf"});
  const std::string kBuildId = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
  const std::string kEmptyBuildId = "";

  SymbolHelper symbol_helper({executable_directory}, {});
  const auto debug_symbol_file_path = symbol_helper.FindDebugSymbolsFile(kFilePath, kBuildId);
  ASSERT_TRUE(debug_symbol_file_path) << debug_symbol_file_path.error().message();
  EXPECT_EQ(debug_symbol_file_path.value(), kFilePath + ".debug");

  const auto result = symbol_helper.FindDebugSymbolsFile(kFilePath, kEmptyBuildId);
  ASSERT_FALSE(result);
  EXPECT_THAT(result.error().message(),
              testing::HasSubstr("Could not find a file with debug symbols"));
}

TEST(SymbolHelper, LoadSymbolsCollectorSeparateFile) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  std::string symbols_file_name = "no_symbols_elf.debug";
  std::string symbols_path = executable_directory + symbols_file_name;

  SymbolHelper symbol_helper({executable_directory}, {});
  const auto symbols_result = symbol_helper.LoadSymbolsCollector(file_path);
  ASSERT_TRUE(symbols_result);
  ModuleSymbols symbols = std::move(symbols_result.value());
  EXPECT_EQ(symbols.symbols_file_path(), symbols_path);
}

TEST(SymbolHelper, LoadUsingSymbolsPathFile) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  std::string symbols_file_name = "no_symbols_elf.debug";
  std::string symbols_path = executable_directory + symbols_file_name;

  SymbolHelper symbol_helper({}, {executable_directory});
  const auto symbols_result =
      symbol_helper.LoadUsingSymbolsPathFile(file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  ASSERT_TRUE(symbols_result);
  ModuleSymbols symbols = std::move(symbols_result.value());

  EXPECT_EQ(symbols.symbols_file_path(), symbols_path);
  EXPECT_FALSE(symbols.symbol_infos().empty());
}

TEST(SymbolHelper, LoadFromFile) {
  const std::string file_path = executable_directory + "no_symbols_elf.debug";
  SymbolHelper symbol_helper;
  const auto symbols_result =
      symbol_helper.LoadSymbolsFromFile(file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");

  ASSERT_TRUE(symbols_result) << symbols_result.error().message();
  ModuleSymbols symbols = std::move(symbols_result.value());

  EXPECT_EQ(symbols.symbols_file_path(), file_path);
  EXPECT_FALSE(symbols.symbol_infos().empty());
}

TEST(SymbolHelper, LoadFromFileInvalidFile) {
  const std::string file_path = executable_directory + "file_does_not_exist";
  SymbolHelper symbol_helper;
  const auto result =
      symbol_helper.LoadSymbolsFromFile(file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");

  ASSERT_FALSE(result);
  EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
              testing::HasSubstr("no such file or directory"));
}

TEST(SymbolHelper, LoadFromFileIvalidBuildId) {
  const std::string file_path = executable_directory + "no_symbols_elf.debug";
  SymbolHelper symbol_helper;
  const auto result = symbol_helper.LoadSymbolsFromFile(file_path, "fish");

  ASSERT_FALSE(result);
  EXPECT_THAT(result.error().message(), testing::HasSubstr("invalid build id"));
}
