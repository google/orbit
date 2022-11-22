// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <optional>
#include <string_view>

#include "AssemblyTestLiterals.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "CodeReport/Disassembler.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "SymbolProvider/ModuleIdentifier.h"

using orbit_code_report::kFibonacciAbsoluteAddress;
using orbit_code_report::kFibonacciAssembly;
using orbit_code_report::kFibonacciDisassembledNoSymbolsLoaded;
using orbit_code_report::kFibonacciDisassembledWithSymbolsLoaded;

TEST(Disassembler, Disassemble) {
  orbit_code_report::Disassembler disassembler{};
  orbit_client_data::ProcessData empty_process{};
  orbit_client_data::ModuleManager empty_module_manager{};
  disassembler.Disassemble(empty_process, empty_module_manager,
                           static_cast<const void*>(kFibonacciAssembly.data()),
                           kFibonacciAssembly.size(), kFibonacciAbsoluteAddress, true);
  EXPECT_EQ(disassembler.GetResult(), kFibonacciDisassembledNoSymbolsLoaded);
  EXPECT_EQ(disassembler.GetAddressAtLine(0), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(4), kFibonacciAbsoluteAddress + 0x0d);
  EXPECT_EQ(disassembler.GetAddressAtLine(12), kFibonacciAbsoluteAddress + 0x1b);
  EXPECT_EQ(disassembler.GetAddressAtLine(24), kFibonacciAbsoluteAddress + 0x37);
  EXPECT_EQ(disassembler.GetAddressAtLine(27), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(28), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(29), 0);  // 29 is the first invalid line index.
  EXPECT_EQ(disassembler.GetAddressAtLine(1024), 0);

  EXPECT_FALSE(disassembler.GetLineAtAddress(0x0).has_value());
  EXPECT_FALSE(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x0c).has_value());
  EXPECT_EQ(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x0d).value_or(0), 4);
  EXPECT_EQ(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x1b).value_or(0), 12);
  EXPECT_EQ(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x37).value_or(0), 24);
  EXPECT_FALSE(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0xdf).has_value());
  EXPECT_FALSE(disassembler.GetLineAtAddress(0x0).has_value());
}

TEST(Disassembler, DisassembleWithSymbols) {
  orbit_code_report::Disassembler disassembler{};

  orbit_grpc_protos::ModuleInfo module_info;
  constexpr uint64_t kOffset = kFibonacciAbsoluteAddress % orbit_module_utils::kPageSize;
  module_info.set_address_start(kFibonacciAbsoluteAddress - kOffset);
  module_info.set_address_end(kFibonacciAbsoluteAddress + kFibonacciAssembly.size());
  constexpr const char* kBuildId = "build_id";
  constexpr const char* kFilePath = "path/to/file";
  module_info.set_build_id(kBuildId);
  module_info.set_file_path(kFilePath);

  orbit_client_data::ProcessData process{};
  process.AddOrUpdateModuleInfo(module_info);

  orbit_client_data::ModuleManager module_manager{};
  (void)module_manager.AddOrUpdateModules({module_info});
  orbit_client_data::ModuleData* module_data = module_manager.GetMutableModuleByModuleIdentifier(
      orbit_symbol_provider::ModuleIdentifier{kFilePath, kBuildId});

  orbit_grpc_protos::ModuleSymbols symbols;
  orbit_grpc_protos::SymbolInfo* symbol = symbols.add_symbol_infos();
  symbol->set_address(kOffset);
  constexpr const char* kDemangledFunctionName = "int fib(int)";
  symbol->set_demangled_name(kDemangledFunctionName);
  symbol->set_size(kFibonacciAssembly.size());
  module_data->AddSymbols(symbols);

  disassembler.Disassemble(process, module_manager,
                           static_cast<const void*>(kFibonacciAssembly.data()),
                           kFibonacciAssembly.size(), kFibonacciAbsoluteAddress, true);
  EXPECT_EQ(disassembler.GetResult(), kFibonacciDisassembledWithSymbolsLoaded);
}