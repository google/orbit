// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "symbol.pb.h"

namespace OrbitClientData {

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::SymbolInfo;

TEST(ModuleManager, GetModuleByPath) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id example";
  uint64_t load_bias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleManager module_manager;
  module_manager.AddOrUpdateModules({module_info});

  const ModuleData* module = module_manager.GetModuleByPath(file_path);
  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), name);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->file_size(), file_size);
  EXPECT_EQ(module->build_id(), build_id);
  EXPECT_EQ(module->load_bias(), load_bias);

  const ModuleData* non_existing_module = module_manager.GetModuleByPath("wrong/path");
  EXPECT_EQ(non_existing_module, nullptr);
}

TEST(ModuleManager, GetMutableModuleByPath) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id example";
  uint64_t load_bias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleManager module_manager;
  module_manager.AddOrUpdateModules({module_info});

  ModuleData* module = module_manager.GetMutableModuleByPath(file_path);
  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), name);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->file_size(), file_size);
  EXPECT_EQ(module->build_id(), build_id);
  EXPECT_EQ(module->load_bias(), load_bias);

  EXPECT_FALSE(module->is_loaded());
  module->AddSymbols({});
  EXPECT_TRUE(module->is_loaded());

  ModuleData* non_existing_module = module_manager.GetMutableModuleByPath("wrong/path");
  EXPECT_EQ(non_existing_module, nullptr);
}

TEST(ModuleManager, AddOrUpdateModules) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id example";
  uint64_t load_bias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleManager module_manager;
  std::vector<ModuleData*> unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_TRUE(unloaded_modules.empty());

  ModuleData* module = module_manager.GetMutableModuleByPath(file_path);

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), name);

  // change name, this updates the module
  std::string changed_name = "changed name";
  module_info.set_name(changed_name);

  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_TRUE(unloaded_modules.empty());

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->name(), changed_name);

  // add symbols
  ModuleSymbols module_symbols;
  module->AddSymbols(module_symbols);
  ASSERT_TRUE(module->is_loaded());

  // change build id, this updates the module and removes the symbols
  module_info.set_build_id("different build id");
  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_FALSE(unloaded_modules.empty());
  EXPECT_EQ(unloaded_modules.size(), 1);

  EXPECT_EQ(module->build_id(), module_info.build_id());
  EXPECT_FALSE(module->is_loaded());

  // change file path & file size and add again (should be added again, because it is now considered
  // a different module)
  std::string different_path = "different/path/of/module";
  module_info.set_file_path(different_path);
  uint64_t different_file_size = 301;
  module_info.set_file_size(different_file_size);
  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_TRUE(unloaded_modules.empty());

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->file_size(), file_size);

  const ModuleData* different_module = module_manager.GetModuleByPath(different_path);
  ASSERT_NE(different_module, nullptr);
  EXPECT_EQ(different_module->file_path(), different_path);
  EXPECT_EQ(different_module->file_size(), different_file_size);
}

TEST(ModuleManager, GetOrbitFunctionsOfProcess) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id example";
  uint64_t load_bias = 0x400;
  uint64_t address_start = 100;
  uint64_t address_end = 1000;

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);
  module_info.set_address_start(address_start);
  module_info.set_address_end(address_end);

  ModuleManager module_manager;
  module_manager.AddOrUpdateModules({module_info});

  ProcessInfo process_info;
  ProcessData process{process_info};

  {
    // process does not have any modules loaded
    std::vector<FunctionInfo> functions = module_manager.GetOrbitFunctionsOfProcess(process);
    EXPECT_TRUE(functions.empty());
  }

  process.UpdateModuleInfos({module_info});
  {
    // process has module loaded, but module does not have symbols loaded
    std::vector<FunctionInfo> functions = module_manager.GetOrbitFunctionsOfProcess(process);
    EXPECT_TRUE(functions.empty());
  }

  ModuleSymbols module_symbols;
  {
    SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
    symbol_info->set_name("mangled_not_an_orbit_function");
    symbol_info->set_demangled_name("not an orbit function");
  }
  {
    SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
    symbol_info->set_name("mangled_orbit_api::Start()");
    symbol_info->set_demangled_name("orbit_api::Start()");
    symbol_info->set_address(500);
  }

  ModuleData* module = module_manager.GetMutableModuleByPath(file_path);
  module->AddSymbols(module_symbols);

  {
    // process has module loaded and module has functions loaded. One of the functions is an orbit
    // function
    std::vector<FunctionInfo> functions = module_manager.GetOrbitFunctionsOfProcess(process);
    ASSERT_EQ(functions.size(), 1);

    FunctionInfo& function{functions[0]};
    EXPECT_EQ(function.name(), "mangled_orbit_api::Start()");
    EXPECT_EQ(function.pretty_name(), "orbit_api::Start()");
    EXPECT_EQ(function.address(), 500);
  }
}

}  // namespace OrbitClientData