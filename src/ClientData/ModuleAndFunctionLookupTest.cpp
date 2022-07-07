// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleManager.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_client_data {

TEST(ModuleAndFunctionLookup, FindFunctionByModulePathBuildIdAndOffset) {
  constexpr const char* kModuleFilePath = "/path/to/module";
  constexpr const char* kModuleBuildId = "build_id";
  constexpr uint64_t kModuleLoadBias = 0x1000;

  constexpr const char* kFunctionName = "foo()";
  constexpr uint64_t kFunctionVirtualAddress = 0x3000;
  constexpr uint64_t kFunctionOffset = kFunctionVirtualAddress - kModuleLoadBias;

  ModuleManager module_manager;

  const FunctionInfo* function_info = FindFunctionByModulePathBuildIdAndOffset(
      module_manager, kModuleFilePath, kModuleBuildId, kFunctionOffset);
  EXPECT_EQ(function_info, nullptr);

  ModuleInfo module_info;
  module_info.set_file_path(kModuleFilePath);
  module_info.set_build_id(kModuleBuildId);
  module_info.set_load_bias(kModuleLoadBias);

  std::ignore = module_manager.AddOrUpdateModules({module_info});

  function_info = FindFunctionByModulePathBuildIdAndOffset(module_manager, kModuleFilePath,
                                                           kModuleBuildId, kFunctionOffset);
  EXPECT_EQ(function_info, nullptr);

  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_demangled_name(kFunctionName);
  symbol_info->set_address(kFunctionVirtualAddress);
  ModuleData* module_data =
      module_manager.GetMutableModuleByPathAndBuildId(kModuleFilePath, kModuleBuildId);
  module_data->AddSymbols(module_symbols);

  function_info = FindFunctionByModulePathBuildIdAndOffset(module_manager, kModuleFilePath,
                                                           kModuleBuildId, kFunctionOffset);
  ASSERT_NE(function_info, nullptr);
  EXPECT_EQ(function_info->pretty_name(), kFunctionName);
  EXPECT_EQ(function_info->address(), kFunctionVirtualAddress);
}

TEST(ModuleAndFunctionLookup, FindFunctionByModulePathBuildIdAndVirtualAddress) {
  constexpr const char* kModuleFilePath = "/path/to/module";
  constexpr const char* kModuleBuildId = "build_id";

  constexpr const char* kFunctionName = "foo()";
  constexpr uint64_t kFunctionVirtualAddress = 0x3000;

  ModuleManager module_manager;

  const FunctionInfo* function_info = FindFunctionByModulePathBuildIdAndVirtualAddress(
      module_manager, kModuleFilePath, kModuleBuildId, kFunctionVirtualAddress);
  EXPECT_EQ(function_info, nullptr);

  ModuleInfo module_info;
  module_info.set_file_path(kModuleFilePath);
  module_info.set_build_id(kModuleBuildId);

  std::ignore = module_manager.AddOrUpdateModules({module_info});

  function_info = FindFunctionByModulePathBuildIdAndVirtualAddress(
      module_manager, kModuleFilePath, kModuleBuildId, kFunctionVirtualAddress);
  EXPECT_EQ(function_info, nullptr);

  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_demangled_name(kFunctionName);
  symbol_info->set_address(kFunctionVirtualAddress);
  ModuleData* module_data =
      module_manager.GetMutableModuleByPathAndBuildId(kModuleFilePath, kModuleBuildId);
  module_data->AddSymbols(module_symbols);

  function_info = FindFunctionByModulePathBuildIdAndVirtualAddress(
      module_manager, kModuleFilePath, kModuleBuildId, kFunctionVirtualAddress);
  ASSERT_NE(function_info, nullptr);
  EXPECT_EQ(function_info->pretty_name(), kFunctionName);
  EXPECT_EQ(function_info->address(), kFunctionVirtualAddress);
}

}  // namespace orbit_client_data
