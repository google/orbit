// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/FunctionUtils.h"

#include <absl/strings/str_join.h>

#include <filesystem>

#include "ObjectUtils/Address.h"
#include "OrbitBase/Logging.h"
#include "absl/strings/match.h"
#include "xxhash.h"

namespace orbit_client_data::function_utils {

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::SymbolInfo;

namespace {
uint64_t StringHash(const std::string& string) {
  return XXH64(string.data(), string.size(), 0xBADDCAFEDEAD10CC);
}
}  // namespace

std::string GetLoadedModuleName(const FunctionInfo& func) {
  return GetLoadedModuleNameByPath(func.module_path());
}

std::string GetLoadedModuleNameByPath(std::string_view module_path) {
  return std::filesystem::path(module_path).filename().string();
}

uint64_t GetHash(const FunctionInfo& func) { return StringHash(func.pretty_name()); }

uint64_t Offset(const FunctionInfo& func, const ModuleData& module) {
  return func.address() - module.load_bias();
}

std::optional<uint64_t> GetAbsoluteAddress(const orbit_client_protos::FunctionInfo& func,
                                           const ProcessData& process, const ModuleData& module) {
  std::vector<uint64_t> page_aligned_base_addresses =
      process.GetModuleBaseAddresses(module.file_path(), module.build_id());

  if (page_aligned_base_addresses.empty()) {
    return std::nullopt;
  }

  if (page_aligned_base_addresses.size() > 1) {
    ERROR(
        "Found multiple mappings for \"%s\" with build_id=%s [%s]: "
        "will use the first one as a base address",
        module.file_path(), module.build_id(),
        absl::StrJoin(page_aligned_base_addresses, ",", [](std::string* out, uint64_t address) {
          return out->append(absl::StrFormat("%#x", address));
        }));
  }

  CHECK(!page_aligned_base_addresses.empty());

  return orbit_object_utils::SymbolVirtualAddressToAbsoluteAddress(
      func.address(), page_aligned_base_addresses.at(0), module.load_bias(),
      module.executable_segment_offset());
}

std::unique_ptr<FunctionInfo> CreateFunctionInfo(const SymbolInfo& symbol_info,
                                                 const std::string& module_path,
                                                 const std::string& module_build_id) {
  auto function_info = std::make_unique<FunctionInfo>();

  function_info->set_name(symbol_info.name());
  function_info->set_pretty_name(symbol_info.demangled_name());
  function_info->set_address(symbol_info.address());
  function_info->set_size(symbol_info.size());
  function_info->set_module_path(module_path);
  function_info->set_module_build_id(module_build_id);
  return function_info;
}

bool IsFunctionSelectable(const FunctionInfo& function) {
  constexpr const char* kLibOrbitUserSpaceInstrumentation = "liborbituserspaceinstrumentation.so";
  return function.module_path().find(kLibOrbitUserSpaceInstrumentation) == std::string::npos;
}

}  // namespace orbit_client_data::function_utils
