// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>

#include "Path.h"
#include "SymbolHelper.h"
#include "symbol.pb.h"

const std::string executable_directory =
    Path::GetExecutablePath() + "testdata/";

TEST(SymbolHelper, LoadSymbolsCollectorSameFile) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  SymbolHelper symbol_helper({executable_directory}, {});
  const auto symbols_result = symbol_helper.LoadSymbolsCollector(file_path);
  ASSERT_TRUE(symbols_result);
  ModuleSymbols symbols = std::move(symbols_result.value());
  EXPECT_EQ(symbols.symbols_file_path(), file_path);
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
  const auto symbols_result = symbol_helper.LoadUsingSymbolsPathFile(
      file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  ASSERT_TRUE(symbols_result);
  ModuleSymbols symbols = std::move(symbols_result.value());

  EXPECT_EQ(symbols.symbols_file_path(), symbols_path);
  EXPECT_FALSE(symbols.symbol_infos().empty());
}
