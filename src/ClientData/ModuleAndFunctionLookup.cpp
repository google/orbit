// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ModuleAndFunctionLookup.h"

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Result.h"

namespace orbit_client_data {
namespace {
using orbit_symbol_provider::ModuleIdentifier;

[[nodiscard]] std::optional<uint64_t>
FindFunctionAbsoluteAddressByInstructionAbsoluteAddressUsingModulesInMemory(
    const ProcessData& process, const ModuleManager& module_manager, uint64_t absolute_address) {
  const auto module_or_error = process.FindModuleByAddress(absolute_address);
  if (module_or_error.has_error()) return std::nullopt;
  const auto& module_in_memory = module_or_error.value();
  const uint64_t module_base_address = module_in_memory.start();

  const ModuleData* module = module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(
      module_in_memory, absolute_address);
  if (module == nullptr) return std::nullopt;

  const uint64_t virtual_address = orbit_module_utils::SymbolAbsoluteAddressToVirtualAddress(
      absolute_address, module_base_address, module->load_bias(),
      module->executable_segment_offset());
  const auto* function_info = module->FindFunctionByVirtualAddress(virtual_address, false);
  if (function_info == nullptr) return std::nullopt;

  return orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
      function_info->address(), module_base_address, module->load_bias(),
      module->executable_segment_offset());
}

[[nodiscard]] std::optional<uint64_t>
FindFunctionAbsoluteAddressByInstructionAbsoluteAddressUsingAddressInfo(
    const CaptureData& capture_data, uint64_t absolute_address) {
  const LinuxAddressInfo* address_info = capture_data.GetAddressInfo(absolute_address);
  if (address_info == nullptr) return std::nullopt;

  return absolute_address - address_info->offset_in_function();
}
}  // namespace

const std::string& GetFunctionNameByAddress(const ModuleManager& module_manager,
                                            const CaptureData& capture_data,
                                            uint64_t absolute_address) {
  const FunctionInfo* function = FindFunctionByAddress(*capture_data.process(), module_manager,
                                                       absolute_address, /*is_exact=*/false);
  if (function != nullptr) {
    return function->pretty_name();
  }
  const LinuxAddressInfo* address_info = capture_data.GetAddressInfo(absolute_address);
  if (address_info == nullptr) {
    return kUnknownFunctionOrModuleName;
  }
  const std::string& function_name = address_info->function_name();
  if (function_name.empty()) {
    return kUnknownFunctionOrModuleName;
  }
  return function_name;
}

// Find the start address of the function this address falls inside. Use the function returned by
// FindFunctionByAddress, and when this fails (e.g., the module containing the function has not
// been loaded) use (for now) the LinuxAddressInfo that is collected for every address in a
// callstack.
std::optional<uint64_t> FindFunctionAbsoluteAddressByInstructionAbsoluteAddress(
    const ModuleManager& module_manager, const CaptureData& capture_data,
    uint64_t absolute_address) {
  auto result = FindFunctionAbsoluteAddressByInstructionAbsoluteAddressUsingModulesInMemory(
      *capture_data.process(), module_manager, absolute_address);
  if (result.has_value()) {
    return result;
  }
  return FindFunctionAbsoluteAddressByInstructionAbsoluteAddressUsingAddressInfo(capture_data,
                                                                                 absolute_address);
}

const FunctionInfo* FindFunctionByModuleIdentifierAndVirtualAddress(
    const ModuleManager& module_manager, const ModuleIdentifier& module_id,
    uint64_t virtual_address) {
  const ModuleData* module_data = module_manager.GetModuleByModuleIdentifier(module_id);
  if (module_data == nullptr) {
    return nullptr;
  }

  return module_data->FindFunctionByVirtualAddress(virtual_address, /*is_exact=*/true);
}

const std::string& GetModulePathByAddress(const ModuleManager& module_manager,
                                          const CaptureData& capture_data,
                                          uint64_t absolute_address) {
  const ModuleData* module_data =
      FindModuleByAddress(*capture_data.process(), module_manager, absolute_address);
  if (module_data != nullptr) {
    return module_data->file_path();
  }

  const LinuxAddressInfo* address_info = capture_data.GetAddressInfo(absolute_address);
  if (address_info == nullptr) {
    return kUnknownFunctionOrModuleName;
  }
  const std::string& module_path = address_info->module_path();
  if (module_path.empty()) {
    return kUnknownFunctionOrModuleName;
  }
  return module_path;
}

std::pair<const std::string&, std::optional<std::string>> FindModulePathAndBuildIdByAddress(
    const ModuleManager& module_manager, const CaptureData& capture_data,
    uint64_t absolute_address) {
  const ModuleData* module_data =
      FindModuleByAddress(*capture_data.process(), module_manager, absolute_address);
  if (module_data != nullptr) {
    return {module_data->file_path(), module_data->build_id()};
  }

  const LinuxAddressInfo* address_info = capture_data.GetAddressInfo(absolute_address);
  if (address_info == nullptr) {
    return {kUnknownFunctionOrModuleName, std::nullopt};
  }
  const std::string& module_path = address_info->module_path();
  if (module_path.empty()) {
    return {kUnknownFunctionOrModuleName, std::nullopt};
  }
  return {module_path, std::nullopt};
}

const FunctionInfo* FindFunctionByAddress(const ProcessData& process,
                                          const ModuleManager& module_manager,
                                          uint64_t absolute_address, bool is_exact) {
  const auto module_or_error = process.FindModuleByAddress(absolute_address);
  if (module_or_error.has_error()) return nullptr;
  const auto& module_in_memory = module_or_error.value();
  const uint64_t module_base_address = module_in_memory.start();

  const ModuleData* module = module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(
      module_in_memory, absolute_address);
  if (module == nullptr) return nullptr;

  const uint64_t virtual_address = orbit_module_utils::SymbolAbsoluteAddressToVirtualAddress(
      absolute_address, module_base_address, module->load_bias(),
      module->executable_segment_offset());
  return module->FindFunctionByVirtualAddress(virtual_address, is_exact);
}

[[nodiscard]] const ModuleData* FindModuleByAddress(const ProcessData& process,
                                                    const ModuleManager& module_manager,
                                                    uint64_t absolute_address) {
  const auto result = process.FindModuleByAddress(absolute_address);
  if (result.has_error()) return nullptr;
  return module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(result.value(),
                                                                    absolute_address);
}

}  // namespace orbit_client_data
