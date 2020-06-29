// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>

#include "OrbitFunction.h"
#include "OrbitModule.h"
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

TEST(SymbolHelper, LoadSymbolsUsingSymbolsFile) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);
  module->m_DebugSignature = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";

  std::string symbols_file_name = "no_symbols_elf.debug";
  std::string symbols_path = executable_directory + symbols_file_name;

  SymbolHelper symbol_helper({}, {executable_directory});
  ASSERT_TRUE(symbol_helper.LoadSymbolsUsingSymbolsFile(module));

  EXPECT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, symbols_path);
  EXPECT_TRUE(module->IsLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), symbols_path);
  EXPECT_EQ(pdb.GetName(), symbols_file_name);
}
