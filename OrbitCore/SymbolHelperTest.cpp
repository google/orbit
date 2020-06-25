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

TEST(SymbolHelper, LoadSymbolsIncludedInBinary) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);

  SymbolHelper symbol_helper;
  ASSERT_TRUE(symbol_helper.LoadSymbolsIncludedInBinary(module));

  EXPECT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, file_path);
  EXPECT_TRUE(module->IsLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), file_path);
  EXPECT_EQ(pdb.GetName(), executable_name);
}

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

TEST(SymbolHelper, LoadSymbolsIntoModule) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  ModuleSymbols module_symbols;
  module_symbols.set_symbols_file_path("path/symbols_file_name");
  module_symbols.set_load_bias(0x400);
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_name("function name");
  symbol_info->set_pretty_name("pretty name");
  symbol_info->set_address(15);
  symbol_info->set_size(12);
  symbol_info->set_source_file("file name");
  symbol_info->set_source_line(70);

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0x40, 0);
  SymbolHelper symbol_helper;
  symbol_helper.LoadSymbolsIntoModule(module, module_symbols);

  ASSERT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, "path/symbols_file_name");
  EXPECT_TRUE(module->IsLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), "path/symbols_file_name");
  EXPECT_EQ(pdb.GetName(), "symbols_file_name");
  EXPECT_EQ(pdb.GetHModule(), 0x40);
  EXPECT_EQ(pdb.GetLoadBias(), 0x400);
  ASSERT_EQ(pdb.GetFunctions().size(), 1);
  auto resulting_function = pdb.GetFunctions()[0];
  EXPECT_EQ(resulting_function->Name(), "function name");
  EXPECT_EQ(resulting_function->PrettyName(), "pretty name");
  EXPECT_EQ(resulting_function->Address(), 15);
  EXPECT_EQ(resulting_function->Size(), 12);
  EXPECT_EQ(resulting_function->File(), "file name");
  EXPECT_EQ(resulting_function->Line(), 70);
}
