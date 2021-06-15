// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "ClientData/FunctionUtils.h"
#include "ClientData/ModuleData.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "symbol.pb.h"

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_client_data {

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
  constexpr const char* kBuildId = "build_id";
  std::string module_file_path = "/test/file/path";
  ModuleInfo module_info{};
  module_info.set_file_path(module_file_path);
  module_info.set_build_id(kBuildId);
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
  module.AddSymbols(module_symbols);
  EXPECT_TRUE(module.is_loaded());

  ASSERT_EQ(module.GetFunctions().size(), 1);

  const FunctionInfo* function = module.GetFunctions()[0];
  EXPECT_EQ(function->name(), symbol_name);
  EXPECT_EQ(function->pretty_name(), symbol_pretty_name);
  EXPECT_EQ(function->module_path(), module_file_path);
  EXPECT_EQ(function->module_build_id(), kBuildId);
  EXPECT_EQ(function->address(), symbol_address);
  EXPECT_EQ(function->size(), symbol_size);
}

TEST(ModuleData, FindFunctionByOffset) {
  uint64_t address1 = 100;
  std::string name1 = "Name 1";
  std::string demangled_name_1 = "Pretty Name 1";
  uint64_t address2 = 200;
  std::string name2 = "Name 2";
  std::string demangled_name_2 = "Pretty Name 2";
  uint64_t size = 10;

  ModuleSymbols symbols;
  SymbolInfo* symbol1 = symbols.add_symbol_infos();
  symbol1->set_name(name1);
  symbol1->set_demangled_name(demangled_name_1);
  symbol1->set_address(address1);
  symbol1->set_size(size);
  SymbolInfo* symbol2 = symbols.add_symbol_infos();
  symbol2->set_name(name2);
  symbol2->set_demangled_name(demangled_name_2);
  symbol2->set_address(address2);
  symbol2->set_size(size);

  ModuleData module{ModuleInfo{}};

  module.AddSymbols(symbols);
  EXPECT_TRUE(module.is_loaded());

  // Find exact
  {
    // address 1
    const FunctionInfo* result = module.FindFunctionByOffset(address1, true);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name1);
  }
  {
    // address 2
    const FunctionInfo* result = module.FindFunctionByOffset(address2, true);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name2);
  }
  {
    // wrong address (larger)
    const FunctionInfo* result = module.FindFunctionByOffset(address1 + 1, true);
    EXPECT_EQ(result, nullptr);
  }
  {
    // wrong address (smaller)
    const FunctionInfo* result = module.FindFunctionByOffset(address1 - 1, true);
    EXPECT_EQ(result, nullptr);
  }

  // Find not exact
  {
    // at address 1
    const FunctionInfo* result = module.FindFunctionByOffset(address1, false);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name1);
  }
  {
    // at address 1 + offset (offset < size)
    uint64_t offset = 5;
    const FunctionInfo* result = module.FindFunctionByOffset(address1 + offset, false);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name(), name1);
  }
  {
    // at address 1 + offset (offset > size)
    uint64_t offset = 15;
    const FunctionInfo* result = module.FindFunctionByOffset(address1 + offset, false);
    EXPECT_EQ(result, nullptr);
  }
  {
    // at address 1 - offset
    uint64_t offset = 5;
    const FunctionInfo* result = module.FindFunctionByOffset(address1 - offset, false);
    EXPECT_EQ(result, nullptr);
  }
}

TEST(ModuleData, FindFunctionFromHash) {
  ModuleSymbols symbols;

  SymbolInfo* symbol = symbols.add_symbol_infos();
  symbol->set_name("Symbol Name");
  symbol->set_demangled_name("demangled name");

  ModuleData module{ModuleInfo{}};
  module.AddSymbols(symbols);

  ASSERT_TRUE(module.is_loaded());
  ASSERT_FALSE(module.GetFunctions().empty());

  const FunctionInfo* function = module.GetFunctions()[0];
  uint64_t hash = function_utils::GetHash(*function);

  {
    const FunctionInfo* result = module.FindFunctionFromHash(hash);
    EXPECT_EQ(result, function);
  }

  {
    const FunctionInfo* result = module.FindFunctionFromHash(hash + 1);
    EXPECT_EQ(result, nullptr);
  }
}

TEST(ModuleData, UpdateIfChanged) {
  std::string name = "Example Name";
  std::string file_path = "/test/file/path";
  uint64_t file_size = 1000;
  std::string build_id{};
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

  module_info.set_name("different name");
  EXPECT_FALSE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.name(), module_info.name());

  module_info.set_file_size(1002);
  EXPECT_FALSE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());

  module_info.set_load_bias(4010);
  EXPECT_FALSE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.load_bias(), module_info.load_bias());

  // add symbols, then change module; symbols are deleted
  ModuleSymbols symbols;
  module.AddSymbols(symbols);
  EXPECT_TRUE(module.is_loaded());

  module_info.set_file_size(1003);
  EXPECT_TRUE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());

  // file_path is not allowed to be changed
  module_info.set_file_path("changed/path");
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");

  // as well as build_id
  module_info.set_build_id("yet another build id");
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");
}

TEST(ModuleData, UpdateIfChangedAndNotLoaded) {
  std::string name = "Example Name";
  std::string file_path = "/test/file/path";
  uint64_t file_size = 1000;
  std::string build_id{};
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

  module_info.set_name("different name");
  EXPECT_TRUE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_EQ(module.name(), module_info.name());

  module_info.set_file_size(1002);
  EXPECT_TRUE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());

  module_info.set_load_bias(4010);
  EXPECT_TRUE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_EQ(module.load_bias(), module_info.load_bias());

  // add symbols, then change module; symbols are deleted
  ModuleSymbols symbols;
  module.AddSymbols(symbols);
  EXPECT_TRUE(module.is_loaded());

  module_info.set_file_size(1003);
  EXPECT_FALSE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_NE(module.file_size(), module_info.file_size());
  EXPECT_TRUE(module.is_loaded());

  // file_path is not allowed to be changed
  module_info.set_file_path("changed/path");
  EXPECT_DEATH((void)module.UpdateIfChangedAndNotLoaded(module_info), "Check failed");

  // as well as build_id
  module_info.set_build_id("yet another build id");
  EXPECT_DEATH((void)module.UpdateIfChangedAndNotLoaded(module_info), "Check failed");
}

TEST(ModuleData, UpdateIfChangedWithBuildId) {
  std::string name = "Example Name";
  std::string file_path = "/test/file/path";
  uint64_t file_size = 1000;
  std::string build_id = "build_id_27";
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

  // We cannot change a module with non-empty build_id
  module_info.set_name("different name");
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");
  EXPECT_DEATH((void)module.UpdateIfChangedAndNotLoaded(module_info), "Check failed");

  // adding symbols should work.
  ModuleSymbols symbols;
  module.AddSymbols(symbols);
  EXPECT_TRUE(module.is_loaded());
}

}  // namespace orbit_client_data
