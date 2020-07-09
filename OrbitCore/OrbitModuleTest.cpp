// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "ElfUtils/ElfFile.h"
#include "OrbitModule.h"
#include "Path.h"
#include "Pdb.h"
#include "SymbolHelper.h"
#include "symbol.pb.h"

const std::string executable_directory =
    Path::GetExecutablePath() + "testdata/";

using ElfUtils::ElfFile;

TEST(OrbitModule, Constructor) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;
  const uint64_t executable_size = 16616;

  uint64_t address_start = 0x700;  // sample test data
  uint64_t address_end = 0x1000;

  Module module(file_path, address_start, address_end);

  EXPECT_EQ(module.m_FullName, file_path);
  EXPECT_EQ(module.m_Name, executable_name);
  EXPECT_EQ(module.m_PdbSize, executable_size);

  EXPECT_EQ(module.m_AddressStart, address_start);
  EXPECT_EQ(module.m_AddressEnd, address_end);

  EXPECT_TRUE(module.IsLoadable());

  EXPECT_EQ(module.m_Pdb, nullptr);
  EXPECT_FALSE(module.IsLoaded());
}

TEST(OrbitModule, LoadFunctions) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);
  {
    const std::unique_ptr<ElfFile> elf_file = ElfFile::Create(file_path);
    const auto symbols = elf_file->LoadSymbols();
    ASSERT_TRUE(symbols);
    module->LoadSymbols(symbols.value());
  }

  ASSERT_TRUE(module->m_Pdb != nullptr);
  Pdb& pdb = *module->m_Pdb;

  // Check functions
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  EXPECT_EQ(functions.size(), 10);
  const Function* function = functions[0].get();

  EXPECT_EQ(function->Name(), "deregister_tm_clones");
  EXPECT_EQ(function->PrettyName(), "deregister_tm_clones");
  EXPECT_EQ(function->Address(), 0x1080);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetLoadedModuleName(), executable_name);

  function = functions[4].get();
  EXPECT_EQ(function->Name(), "_init");
  EXPECT_EQ(function->PrettyName(), "_init");
  EXPECT_EQ(function->Address(), 0x1000);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetLoadedModuleName(), executable_name);

  function = functions[9].get();
  EXPECT_EQ(function->Name(), "main");
  EXPECT_EQ(function->PrettyName(), "main");
  EXPECT_EQ(function->Address(), 0x1135);
  EXPECT_EQ(function->Size(), 35);
  EXPECT_EQ(function->GetLoadedModuleName(), executable_name);
}

TEST(OrbitModule, GetFunctionFromExactAddress) {
  const std::string file_path = executable_directory + "hello_world_static_elf";

  std::shared_ptr<Module> module =
      std::make_shared<Module>(file_path, 0x400000, 0);
  {
    const std::unique_ptr<ElfFile> elf_file = ElfFile::Create(file_path);
    const auto symbols = elf_file->LoadSymbols();
    ASSERT_TRUE(symbols);
    module->LoadSymbols(symbols.value());
  }

  ASSERT_TRUE(module->m_Pdb != nullptr);
  Pdb& pdb = *module->m_Pdb;

  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;
  const Function* function = pdb.GetFunctionFromExactAddress(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");

  EXPECT_EQ(pdb.GetFunctionFromExactAddress(__free_pc_addr), nullptr);
}

TEST(OrbitModule, GetFunctionFromProgramCounter) {
  const std::string file_path = executable_directory + "hello_world_static_elf";

  std::shared_ptr<Module> module =
      std::make_shared<Module>(file_path, 0x400000, 0);
  {
    const std::unique_ptr<ElfFile> elf_file = ElfFile::Create(file_path);
    const auto symbols = elf_file->LoadSymbols();
    ASSERT_TRUE(symbols);
    module->LoadSymbols(symbols.value());
  }

  ASSERT_TRUE(module->m_Pdb != nullptr);
  Pdb& pdb = *module->m_Pdb;

  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();

  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;

  const Function* function =
      pdb.GetFunctionFromProgramCounter(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");

  function = pdb.GetFunctionFromProgramCounter(__free_pc_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");
}

TEST(SymbolHelper, LoadSymbols) {
  ModuleSymbols module_symbols;
  module_symbols.set_symbols_file_path("path/symbols_file_name");
  module_symbols.set_load_bias(0x400);
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_name("function name");
  symbol_info->set_demangled_name("pretty name");
  symbol_info->set_address(15);
  symbol_info->set_size(12);
  symbol_info->set_source_file("file name");
  symbol_info->set_source_line(70);

  std::shared_ptr<Module> module =
      std::make_shared<Module>("module name", 0x40, 0);
  module->LoadSymbols(module_symbols);

  ASSERT_NE(module->m_Pdb, nullptr);
  EXPECT_TRUE(module->IsLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), "module name");
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
