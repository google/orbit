// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <string>
#include <tuple>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "SymbolProvider/ModuleIdentifier.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_client_data {

TEST(ModuleAndFunctionLookup, FindFunctionByModulePathBuildIdAndVirtualAddress) {
  constexpr const char* kModuleFilePath = "/path/to/module";
  constexpr const char* kModuleBuildId = "build_id";
  const orbit_symbol_provider::ModuleIdentifier module_id{kModuleFilePath, kModuleBuildId};

  constexpr const char* kFunctionName = "foo()";
  constexpr uint64_t kFunctionVirtualAddress = 0x3000;

  ModuleManager module_manager;

  const FunctionInfo* function_info = FindFunctionByModuleIdentifierAndVirtualAddress(
      module_manager, module_id, kFunctionVirtualAddress);
  EXPECT_EQ(function_info, nullptr);

  ModuleInfo module_info;
  module_info.set_file_path(kModuleFilePath);
  module_info.set_build_id(kModuleBuildId);

  std::ignore = module_manager.AddOrUpdateModules({module_info});

  function_info = FindFunctionByModuleIdentifierAndVirtualAddress(module_manager, module_id,
                                                                  kFunctionVirtualAddress);
  EXPECT_EQ(function_info, nullptr);

  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_demangled_name(kFunctionName);
  symbol_info->set_address(kFunctionVirtualAddress);
  ModuleData* module_data = module_manager.GetMutableModuleByModuleIdentifier(module_id);
  module_data->AddSymbols(module_symbols);

  function_info = FindFunctionByModuleIdentifierAndVirtualAddress(module_manager, module_id,
                                                                  kFunctionVirtualAddress);
  ASSERT_NE(function_info, nullptr);
  EXPECT_EQ(function_info->pretty_name(), kFunctionName);
  EXPECT_EQ(function_info->address(), kFunctionVirtualAddress);
}

}  // namespace orbit_client_data
