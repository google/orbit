// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitModule.h"

#include <absl/strings/str_format.h>

#include <cinttypes>
#include <string>

#include "FunctionUtils.h"
#include "OrbitBase/Logging.h"
#include "Path.h"
#include "Pdb.h"
#include "capture_data.pb.h"
#include "symbol.pb.h"

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

void Module::LoadSymbols(const ModuleSymbols& module_symbols) {
  if (m_Pdb != nullptr) {
    LOG("Warning: Module \"%s\" already contained symbols, will override now", m_Name);
  }

  m_Pdb = std::make_shared<Pdb>(m_AddressStart, module_symbols.load_bias(), m_FullName);

  for (const SymbolInfo& symbol_info : module_symbols.symbol_infos()) {
    std::shared_ptr<FunctionInfo> function = FunctionUtils::CreateFunctionInfo(
        symbol_info.name(), symbol_info.demangled_name(), symbol_info.address(),
        module_symbols.load_bias(), symbol_info.size(), "", 0, m_FullName, m_AddressStart);
    m_Pdb->AddFunction(function);
  }

  m_Pdb->ProcessData();
  SetLoaded(true);
}
