// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitModule.h"

#include <absl/strings/str_format.h>

#include <cinttypes>
#include <string>

#include "Core.h"
#include "ElfUtils/ElfFile.h"
#include "FunctionUtils.h"
#include "OrbitBase/Logging.h"
#include "Pdb.h"
#include "function.pb.h"
#include "symbol.pb.h"

#ifndef WIN32
#include "LinuxUtils.h"
#include "Params.h"
#include "Path.h"
#endif

//-----------------------------------------------------------------------------
Module::Module(const std::string& file_name, uint64_t address_start,
               uint64_t address_end) {
  if (!Path::FileExists(file_name)) {
    ERROR("Creating Module from path \"%s\": file does not exist",
          file_name.c_str());
  }

  m_FullName = file_name;
  m_Name = Path::GetFileName(file_name);
  m_PdbSize = Path::FileSize(file_name);

  m_AddressStart = address_start;
  m_AddressEnd = address_end;

  loadable_ = true;  // necessary, because it toggles "Load Symbols" option
}

//-----------------------------------------------------------------------------
void Module::LoadSymbols(const ModuleSymbols& module_symbols) {
  if (m_Pdb != nullptr) {
    LOG("Warning: Module %s already contained symbols, will override now.",
        m_Name);
  }

  m_Pdb = std::make_shared<Pdb>(m_AddressStart, module_symbols.load_bias(),
                                module_symbols.symbols_file_path(), m_FullName);

  for (const SymbolInfo& symbol_info : module_symbols.symbol_infos()) {
    std::shared_ptr<Function> function = FunctionUtils::CreateFunction(
        symbol_info.name(), symbol_info.demangled_name(), symbol_info.address(),
        module_symbols.load_bias(), symbol_info.size(),
        symbol_info.source_file(), symbol_info.source_line(), m_FullName,
        m_AddressStart);
    m_Pdb->AddFunction(function);
  }

  m_Pdb->ProcessData();
  SetLoaded(true);
}
