// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitProcess.h"

#include <OrbitBase/Logging.h>
#include <absl/strings/ascii.h>

#include "OrbitModule.h"
#include "Path.h"
#include "Pdb.h"

using orbit_client_protos::FunctionInfo;

Process::Process() {
  id_ = -1;
  is_64_bit_ = false;
}

FunctionInfo* Process::GetFunctionFromAddress(uint64_t address, bool a_IsExact) {
  if (m_Modules.empty()) {
    return nullptr;
  }

  auto it = m_Modules.upper_bound(address);
  if (it == m_Modules.begin()) {
    return nullptr;
  }

  --it;
  std::shared_ptr<Module>& module = it->second;
  if (address >= module->m_AddressEnd) {
    return nullptr;
  }

  if (module->m_Pdb == nullptr) {
    return nullptr;
  }

  if (a_IsExact) {
    return module->m_Pdb->GetFunctionFromExactAddress(address);
  } else {
    return module->m_Pdb->GetFunctionFromProgramCounter(address);
  }
}

std::shared_ptr<Module> Process::GetModuleFromAddress(uint64_t a_Address) {
  if (m_Modules.empty()) {
    return nullptr;
  }

  auto module_it = m_Modules.upper_bound(a_Address);
  if (module_it == m_Modules.begin()) {
    return nullptr;
  }

  --module_it;
  std::shared_ptr<Module> module = module_it->second;
  CHECK(a_Address >= module->m_AddressStart);
  if (a_Address >= module->m_AddressEnd) {
    return nullptr;
  }

  return module;
}

std::shared_ptr<Module> Process::GetModuleFromName(const std::string& a_Name) {
  auto iter = m_NameToModuleMap.find(absl::AsciiStrToLower(a_Name));
  if (iter != m_NameToModuleMap.end()) {
    return iter->second;
  }

  return nullptr;
}

std::shared_ptr<Module> Process::GetModuleFromPath(const std::string& module_path) {
  auto iter = path_to_module_map_.find(module_path);
  if (iter != path_to_module_map_.end()) {
    return iter->second;
  }

  return nullptr;
}

void Process::AddFunctions(
    const std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>& functions) {
  ScopeLock lock(data_mutex_);
  for (const auto& function : functions) {
    AddFunction(function);
  }
}

void Process::AddModule(std::shared_ptr<Module>& a_Module) {
  m_Modules[a_Module->m_AddressStart] = a_Module;
  m_NameToModuleMap[absl::AsciiStrToLower(a_Module->m_Name)] = a_Module;
  path_to_module_map_[a_Module->m_FullName] = a_Module;
}
