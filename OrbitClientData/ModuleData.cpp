// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/ModuleData.h"

#include "OrbitBase/Logging.h"
#include "OrbitClientData/FunctionUtils.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"
#include "module.pb.h"

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleInfo;

bool ModuleData::is_loaded() const {
  absl::MutexLock lock(&mutex_);
  return is_loaded_;
}

void ModuleData::UpdateIfChanged(ModuleInfo info) {
  absl::MutexLock lock(&mutex_);

  CHECK(file_path() == info.file_path());

  // TODO(171878807): Remove this as soon as a better way of distinguishing modules is implemented.
  const bool build_id_matching = !build_id().empty() && build_id() == info.build_id();
  const bool all_module_properties_matching =
      build_id() == info.build_id() && name() == info.name() && file_size() == info.file_size() &&
      load_bias() == info.load_bias();

  module_info_ = std::move(info);

  if (build_id_matching) return;

  if (all_module_properties_matching) return;

  LOG("Module %s changed.", file_path());

  if (!is_loaded_) return;

  LOG("Module %s contained symbols. Because the module changed, those are now removed.",
      file_path());
  functions_.clear();
  hash_to_function_map_.clear();
  is_loaded_ = false;
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

void ModuleData::AddSymbols(const orbit_grpc_protos::ModuleSymbols& module_symbols) {
  absl::MutexLock lock(&mutex_);
  CHECK(!is_loaded_);

  uint32_t address_reuse_counter = 0;
  for (const orbit_grpc_protos::SymbolInfo& symbol_info : module_symbols.symbol_infos()) {
    auto [inserted_it, success] = functions_.try_emplace(
        symbol_info.address(), FunctionUtils::CreateFunctionInfo(symbol_info, file_path()));
    FunctionInfo* function = inserted_it->second.get();
    // It happens that the same address has multiple symbol names associated
    // with it. For example: (all the same address)
    // __cxxabiv1::__enum_type_info::~__enum_type_info()
    // __cxxabiv1::__shim_type_info::~__shim_type_info()
    // __cxxabiv1::__array_type_info::~__array_type_info()
    // __cxxabiv1::__class_type_info::~__class_type_info()
    // __cxxabiv1::__pbase_type_info::~__pbase_type_info()
    if (success) {
      const bool success =
          hash_to_function_map_.try_emplace(FunctionUtils::GetHash(*function), function).second;
      if (!success) {
        LOG("Warning: Multiple functions with the same demangled name: %s (this is currently not "
            "supported by presets)",
            function->pretty_name());
      }
    } else {
      address_reuse_counter++;
    }
  }
  if (address_reuse_counter != 0) {
    LOG("Warning: %d absolute addresses are used by more than one symbol", address_reuse_counter);
  }

  is_loaded_ = true;
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
  for (const auto& [_, function] : functions_) {
    if (FunctionUtils::IsOrbitFunc(*function)) {
      result.emplace_back(*function);
    }
  }
  return result;
}
