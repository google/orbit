// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/ModuleData.h"

#include "OrbitBase/Logging.h"
#include "OrbitClientData/FunctionUtils.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"

using orbit_client_protos::FunctionInfo;

bool ModuleData::is_loaded() const {
  absl::MutexLock lock(&mutex_);
  return is_loaded_;
}

const orbit_client_protos::FunctionInfo* ModuleData::FindFunctionByRelativeAddress(
    uint64_t relative_address, bool is_exact) const {
  uint64_t elf_address = relative_address + load_bias();
  return FindFunctionByElfAddress(elf_address, is_exact);
}

const FunctionInfo* ModuleData::FindFunctionByElfAddress(uint64_t elf_address,
                                                         bool is_exact) const {
  absl::MutexLock lock(&mutex_);
  if (functions_.empty()) return nullptr;

  if (is_exact) {
    auto it = functions_.find(elf_address);
    return (it != functions_.end()) ? it->second.get() : nullptr;
  }

  auto it = functions_.upper_bound(elf_address);
  if (it == functions_.begin()) return nullptr;

  --it;
  FunctionInfo* function = it->second.get();
  CHECK(function->address() <= elf_address);

  if (function->address() + function->size() < elf_address) return nullptr;

  return function;
}

void ModuleData::AddSymbols(const orbit_grpc_protos::ModuleSymbols& module_symbols,
                            uint64_t module_base_address) {
  absl::MutexLock lock(&mutex_);
  CHECK(!is_loaded_);

  uint32_t address_reuse_counter = 0;
  for (const orbit_grpc_protos::SymbolInfo& symbol_info : module_symbols.symbol_infos()) {
    auto [inserted_it, success] = functions_.try_emplace(
        symbol_info.address(), FunctionUtils::CreateFunctionInfo(symbol_info, load_bias(),
                                                                 file_path(), module_base_address));
    FunctionInfo* function = inserted_it->second.get();
    // It happens that the same address has multiple symbol names associated
    // with it. For example: (all the same address)
    // __cxxabiv1::__enum_type_info::~__enum_type_info()
    // __cxxabiv1::__shim_type_info::~__shim_type_info()
    // __cxxabiv1::__array_type_info::~__array_type_info()
    // __cxxabiv1::__class_type_info::~__class_type_info()
    // __cxxabiv1::__pbase_type_info::~__pbase_type_info()
    if (success) {
      hash_to_function_map_.emplace(FunctionUtils::GetHash(*function), function);
    } else {
      address_reuse_counter++;
    }
  }
  if (address_reuse_counter != 0) {
    LOG("Warning: %d absolute addresses are used by more than one symbol", address_reuse_counter);
  }

  is_loaded_ = true;
}

void ModuleData::UpdateFunctionsModuleBaseAddress(uint64_t module_base_address) {
  absl::MutexLock lock(&mutex_);
  for (auto& [_, function] : functions_) {
    function->set_module_base_address(module_base_address);
  }
}

const orbit_client_protos::FunctionInfo* ModuleData::FindFunctionFromHash(uint64_t hash) const {
  absl::MutexLock lock(&mutex_);
  return hash_to_function_map_.contains(hash) ? hash_to_function_map_.at(hash) : nullptr;
}

const std::vector<const FunctionInfo*> ModuleData::GetFunctions() const {
  absl::MutexLock lock(&mutex_);
  std::vector<const FunctionInfo*> result;
  result.reserve(functions_.size());
  for (const auto& pair : functions_) {
    result.push_back(pair.second.get());
  }
  return result;
}

std::vector<FunctionInfo> ModuleData::GetOrbitFunctions() const {
  absl::MutexLock lock(&mutex_);
  CHECK(is_loaded_);
  std::vector<FunctionInfo> result;
  for (const auto& pair : functions_) {
    FunctionInfo function = *pair.second;
    if (FunctionUtils::IsOrbitFunc(function)) {
      result.emplace_back(std::move(function));
    }
  }
  return result;
}
