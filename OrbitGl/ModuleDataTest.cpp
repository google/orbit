// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include "FunctionUtils.h"
#include "ModuleData.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "symbol.pb.h"

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

TEST(ModuleData, Constructor) {
  std::string name = "Example Name";
  std::string file_path = "/test/file/path";
  uint64_t file_size = 1000;
  std::string build_id = "test build id";
  uint64_t load_bias = 4000;

  ModuleInfo module_info{};
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleData module{module_info};

  EXPECT_EQ(module.name(), name);
  EXPECT_EQ(module.file_path(), file_path);
  EXPECT_EQ(module.file_size(), file_size);
  EXPECT_EQ(module.build_id(), build_id);
  EXPECT_EQ(module.load_bias(), load_bias);
  EXPECT_FALSE(module.is_loaded());
  EXPECT_TRUE(module.GetFunctions().empty());
}

TEST(ModuleData, LoadSymbols) {
  // Setup ModuleData
  std::string module_file_path = "/test/file/path";
  uint64_t module_base_start = 0x40;
  uint64_t module_load_bias = 0x400;
  ModuleInfo module_info{};
  module_info.set_file_path(module_file_path);
  module_info.set_load_bias(module_load_bias);
  ModuleData module{module_info};

  // Setup ModuleSymbols
  std::string symbol_name = "function name";
  auto symbol_pretty_name = "pretty name";
  uint64_t symbol_address = 15;
  uint64_t symbol_size = 12;

  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_name(symbol_name);
  symbol_info->set_demangled_name(symbol_pretty_name);
  symbol_info->set_address(symbol_address);
  symbol_info->set_size(symbol_size);

  // Test
  module.AddSymbols(module_symbols, module_base_start);
  EXPECT_TRUE(module.is_loaded());

  ASSERT_EQ(module.GetFunctions().size(), 1);

  const FunctionInfo* function = module.GetFunctions()[0];
  EXPECT_EQ(function->name(), symbol_name);
  EXPECT_EQ(function->pretty_name(), symbol_pretty_name);
  EXPECT_EQ(function->loaded_module_path(), module_file_path);
  EXPECT_EQ(function->module_base_address(), module_base_start);
  EXPECT_EQ(function->address(), symbol_address);
  EXPECT_EQ(function->load_bias(), module_load_bias);
  EXPECT_EQ(function->size(), symbol_size);
  EXPECT_EQ(function->file(), "");
  EXPECT_EQ(function->line(), 0);
}

TEST(ModuleData, ClearSymbols) {
  ModuleData module{ModuleInfo{}};
  module.AddSymbols(ModuleSymbols(), 0);
  EXPECT_TRUE(module.is_loaded());
  module.ClearSymbols();
  EXPECT_FALSE(module.is_loaded());
  EXPECT_EQ(module.GetFunctions().size(), 0);

  // don't clear when no symbols are loaded
  EXPECT_DEATH(module.ClearSymbols(), "Check failed");
}

TEST(ModuleData, FindFunctionByRelativeAddress) {
  uint64_t address1 = 100;
  std::string name1 = "Name 1";
  uint64_t address2 = 200;
  std::string name2 = "Name 2";
  uint64_t size = 10;

  ModuleSymbols symbols;
  SymbolInfo* symbol1 = symbols.add_symbol_infos();
  symbol1->set_name(name1);
  symbol1->set_address(address1);
  symbol1->set_size(size);
  SymbolInfo* symbol2 = symbols.add_symbol_infos();
  symbol2->set_name(name2);
  symbol2->set_address(address2);
  symbol2->set_size(size);

  ModuleData module{ModuleInfo{}};

  module.AddSymbols(symbols, 1000);
  EXPECT_TRUE(module.is_loaded());

  // Find exact
  {
    // address 1
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1, true);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name1);
  }
  {
    // address 2
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address2, true);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name2);
  }
  {
    // wrong address (larger)
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1 + 1, true);
    EXPECT_EQ(result, nullptr);
  }
  {
    // wrong address (smaller)
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1 - 1, true);
    EXPECT_EQ(result, nullptr);
  }

  // Find not exact
  {
    // at address 1
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1, false);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name1);
  }
  {
    // at address 1 + offset (offset < size)
    uint64_t offset = 5;
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1 + offset, false);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name1);
  }
  {
    // at address 1 + offset (offset > size)
    uint64_t offset = 15;
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1 + offset, false);
    EXPECT_EQ(result, nullptr);
  }
  {
    // at address 1 - offset
    uint64_t offset = 5;
    const FunctionInfo* result = module.FindFunctionByRelativeAddress(address1 - offset, false);
    EXPECT_EQ(result, nullptr);
  }
}

TEST(ModuleData, FindFunctionFromHash) {
  ModuleSymbols symbols;

  SymbolInfo* symbol = symbols.add_symbol_infos();
  symbol->set_name("Symbol Name");

  ModuleData module{ModuleInfo{}};
  module.AddSymbols(symbols, 0);

  ASSERT_TRUE(module.is_loaded());
  ASSERT_FALSE(module.GetFunctions().empty());

  const FunctionInfo* function = module.GetFunctions()[0];
  uint64_t hash = FunctionUtils::GetHash(*function);

  {
    const FunctionInfo* result = module.FindFunctionFromHash(hash);
    EXPECT_EQ(result, function);
  }

  {
    const FunctionInfo* result = module.FindFunctionFromHash(hash + 1);
    EXPECT_EQ(result, nullptr);
  }
}