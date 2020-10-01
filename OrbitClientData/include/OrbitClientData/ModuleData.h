// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MODULE_DATA_H_
#define ORBIT_GL_MODULE_DATA_H_

#include <cinttypes>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "symbol.pb.h"

// Represents information about module on the client
class ModuleData final {
 public:
  explicit ModuleData(orbit_grpc_protos::ModuleInfo info)
      : module_info_(std::move(info)), is_loaded_(false) {}

  [[nodiscard]] const std::string& name() const { return module_info_.name(); }
  [[nodiscard]] const std::string& file_path() const { return module_info_.file_path(); }
  [[nodiscard]] uint64_t file_size() const { return module_info_.file_size(); }
  [[nodiscard]] const std::string& build_id() const { return module_info_.build_id(); }
  [[nodiscard]] uint64_t load_bias() const { return module_info_.load_bias(); }
  [[nodiscard]] bool is_loaded() const;
  [[nodiscard]] const orbit_grpc_protos::ModuleInfo& module_info() const { return module_info_; }
  // relative_address here is the absolute address minus the address this module was loaded at by
  // the process (module base address)
  [[nodiscard]] const orbit_client_protos::FunctionInfo* FindFunctionByRelativeAddress(
      uint64_t relative_address, bool is_exact) const;
  [[nodiscard]] const orbit_client_protos::FunctionInfo* FindFunctionByElfAddress(
      uint64_t elf_address, bool is_exact) const;
  // TODO(169309553): The module_base_address parameter should not be needed here, but it is
  // because FunctionInfo still contains the field module_base_address. As soon as that field is
  // gone, remove the parameter here
  void AddSymbols(const orbit_grpc_protos::ModuleSymbols& module_symbols,
                  uint64_t module_base_address);
  // TODO(169309553): As soon as FunctionInfo does not contain module_base_address anymore,
  // completely remove the following method
  void UpdateFunctionsModuleBaseAddress(uint64_t module_base_address);
  [[nodiscard]] const orbit_client_protos::FunctionInfo* FindFunctionFromHash(uint64_t hash) const;
  [[nodiscard]] const std::vector<const orbit_client_protos::FunctionInfo*> GetFunctions() const;
  [[nodiscard]] std::vector<orbit_client_protos::FunctionInfo> GetOrbitFunctions() const;

 private:
  mutable absl::Mutex mutex_;
  const orbit_grpc_protos::ModuleInfo module_info_;
  bool is_loaded_;
  std::map<uint64_t, std::unique_ptr<orbit_client_protos::FunctionInfo>> functions_;
  // TODO(168799822) This is a map of hash to function used for preset loading. Currently presets
  // are based on a hash of the functions pretty name. This should be changed to not use hashes
  // anymore.
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo*> hash_to_function_map_;
};

#endif  // ORBIT_GL_MODULE_DATA_H_
