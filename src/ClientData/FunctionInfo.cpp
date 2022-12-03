// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/FunctionInfo.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <xxhash.h>

#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

uint64_t FunctionInfo::GetPrettyNameHash() const {
  return XXH64(pretty_name_.data(), pretty_name_.size(), 0xBADDCAFEDEAD10CC);
}

uint64_t FunctionInfo::ComputeFileOffset(const ModuleData& module) const {
  return module.ConvertFromVirtualAddressToOffsetInFile(address());
}

std::optional<uint64_t> FunctionInfo::GetAbsoluteAddress(const ProcessData& process,
                                                         const ModuleData& module) const {
  std::vector<uint64_t> page_aligned_base_addresses =
      process.GetModuleBaseAddresses(module.file_path(), module.build_id());

  if (page_aligned_base_addresses.empty()) {
    return std::nullopt;
  }

  if (page_aligned_base_addresses.size() > 1) {
    ORBIT_ERROR(
        "Found multiple mappings for \"%s\" with build_id=%s [%s]: "
        "will use the first one as a base address",
        module.file_path(), module.build_id(),
        absl::StrJoin(page_aligned_base_addresses, ",", [](std::string* out, uint64_t address) {
          return out->append(absl::StrFormat("%#x", address));
        }));
  }

  ORBIT_CHECK(!page_aligned_base_addresses.empty());

  return orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
      address(), page_aligned_base_addresses.at(0), module.load_bias(),
      module.executable_segment_offset());
}

bool FunctionInfo::IsFunctionSelectable() const {
  constexpr const char* kLibOrbitUserSpaceInstrumentation = "liborbituserspaceinstrumentation.so";
  if (absl::StrContains(module_path(), kLibOrbitUserSpaceInstrumentation)) {
    return false;
  }

  constexpr const char* kNameOfWineSyscallDispatcher = "__wine_syscall_dispatcher";
  constexpr const char* kNameOfWineSyscallDispatcherModule = "ntdll.so";
  if (absl::StrContains(pretty_name(), kNameOfWineSyscallDispatcher) &&
      absl::StrContains(module_path(), kNameOfWineSyscallDispatcherModule)) {
    return false;
  }

  return true;
}

}  // namespace orbit_client_data
