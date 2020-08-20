// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "ElfUtils/ElfFile.h"
#include "FunctionUtils.h"
#include "OrbitModule.h"
#include "Path.h"
#include "Pdb.h"
#include "SymbolHelper.h"
#include "capture_data.pb.h"
#include "symbol.pb.h"

const std::string executable_directory = Path::GetExecutablePath() + "testdata/";

using ElfUtils::ElfFile;
using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

TEST(OrbitModule, LoadFunctions) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>();
  module->m_FullName = file_path;
  {
    ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file = ElfFile::Create(file_path);
    ASSERT_TRUE(elf_file) << elf_file.error().message();
    const auto symbols = elf_file.value()->LoadSymbols();
    ASSERT_TRUE(symbols) << symbols.error().message();
    module->LoadSymbols(symbols.value());
  }

  ASSERT_TRUE(module->m_Pdb != nullptr);
  Pdb& pdb = *module->m_Pdb;

  // Check functions
  const std::vector<std::shared_ptr<FunctionInfo>>& functions = pdb.GetFunctions();

  EXPECT_EQ(functions.size(), 10);
  const FunctionInfo* function = functions[0].get();

  EXPECT_EQ(function->name(), "deregister_tm_clones");
  EXPECT_EQ(function->pretty_name(), "deregister_tm_clones");
  EXPECT_EQ(function->address(), 0x1080);
  EXPECT_EQ(function->size(), 0);
  EXPECT_EQ(FunctionUtils::GetLoadedModuleName(*function), executable_name);

  function = functions[4].get();
  EXPECT_EQ(function->name(), "_init");
  EXPECT_EQ(function->pretty_name(), "_init");
  EXPECT_EQ(function->address(), 0x1000);
  EXPECT_EQ(function->size(), 0);
  EXPECT_EQ(FunctionUtils::GetLoadedModuleName(*function), executable_name);

  function = functions[9].get();
  EXPECT_EQ(function->name(), "main");
  EXPECT_EQ(function->pretty_name(), "main");
  EXPECT_EQ(function->address(), 0x1135);
  EXPECT_EQ(function->size(), 35);
  EXPECT_EQ(FunctionUtils::GetLoadedModuleName(*function), executable_name);
}

TEST(OrbitModule, GetFunctionFromExactAddress) {
  const std::string file_path = executable_directory + "hello_world_static_elf";

  std::shared_ptr<Module> module = std::make_shared<Module>();
  module->m_FullName = file_path;
  module->m_AddressStart = 0x400000;
  {
    ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file = ElfFile::Create(file_path);
    ASSERT_TRUE(elf_file) << elf_file.error().message();
    const auto symbols = elf_file.value()->LoadSymbols();
    ASSERT_TRUE(symbols) << symbols.error().message();
    module->LoadSymbols(symbols.value());
  }

  ASSERT_TRUE(module->m_Pdb != nullptr);
  Pdb& pdb = *module->m_Pdb;

  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();
  const std::vector<std::shared_ptr<FunctionInfo>>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;
  const FunctionInfo* function = pdb.GetFunctionFromExactAddress(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->name(), "__free");

  EXPECT_EQ(pdb.GetFunctionFromExactAddress(__free_pc_addr), nullptr);
}

TEST(OrbitModule, GetFunctionFromProgramCounter) {
  const std::string file_path = executable_directory + "hello_world_static_elf";

  std::shared_ptr<Module> module = std::make_shared<Module>();
  module->m_FullName = file_path;
  module->m_AddressStart = 0x400000;
  {
    ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file = ElfFile::Create(file_path);
    ASSERT_TRUE(elf_file) << elf_file.error().message();
    const auto symbols = elf_file.value()->LoadSymbols();
    ASSERT_TRUE(symbols) << symbols.error().message();
    module->LoadSymbols(symbols.value());
  }

  ASSERT_TRUE(module->m_Pdb != nullptr);
  Pdb& pdb = *module->m_Pdb;

  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();

  const std::vector<std::shared_ptr<FunctionInfo>>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;

  const FunctionInfo* function = pdb.GetFunctionFromProgramCounter(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->name(), "__free");

  function = pdb.GetFunctionFromProgramCounter(__free_pc_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->name(), "__free");
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

  std::shared_ptr<Module> module = std::make_shared<Module>();
  module->m_FullName = "module name";
  module->m_AddressStart = 0x40;

  module->LoadSymbols(module_symbols);

  ASSERT_NE(module->m_Pdb, nullptr);
  EXPECT_TRUE(module->IsLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), "module name");
  EXPECT_EQ(pdb.GetHModule(), 0x40);
  EXPECT_EQ(pdb.GetLoadBias(), 0x400);
  ASSERT_EQ(pdb.GetFunctions().size(), 1);
  auto resulting_function = pdb.GetFunctions()[0];
  EXPECT_EQ(resulting_function->name(), "function name");
  EXPECT_EQ(resulting_function->pretty_name(), "pretty name");
  EXPECT_EQ(resulting_function->address(), 15);
  EXPECT_EQ(resulting_function->size(), 12);
  EXPECT_EQ(resulting_function->file(), "file name");
  EXPECT_EQ(resulting_function->line(), 70);
}
