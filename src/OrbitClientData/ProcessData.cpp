// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "OrbitClientData/ProcessData.h"

#include <absl/container/flat_hash_map.h>

#include <cstdint>
#include <vector>

#include "OrbitBase/Result.h"
#include "absl/strings/str_format.h"
#include "process.pb.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::ProcessInfo;

ProcessData::ProcessData() { process_info_.set_pid(-1); }

void ProcessData::UpdateModuleInfos(absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  module_memory_map_.clear();
  start_addresses_.clear();

  for (const auto& module_info : module_infos) {
    {
      const auto [it, success] = module_memory_map_.try_emplace(
          module_info.file_path(),
          MemorySpace{module_info.address_start(), module_info.address_end()});
      CHECK(success);
    }
    {
      const auto [it, success] =
          start_addresses_.try_emplace(module_info.address_start(), module_info.file_path());
      CHECK(success);
    }
  }
}

ErrorMessageOr<std::pair<std::string, uint64_t>> ProcessData::FindModuleByAddress(
    uint64_t absolute_address) const {
  if (start_addresses_.empty()) {
    return ErrorMessage(absl::StrFormat("Unable to find module for address %016" PRIx64
                                        ": No modules loaded by process %s",
                                        absolute_address, name()));
  }

  ErrorMessage not_found_error =
      ErrorMessage(absl::StrFormat("Unable to find module for address %016" PRIx64
                                   ": No module loaded at this address by process %s",
                                   absolute_address, name()));

  auto it = start_addresses_.upper_bound(absolute_address);
  if (it == start_addresses_.begin()) return not_found_error;

  --it;
  const std::string& module_path = it->second;
  const MemorySpace& memory_space = module_memory_map_.at(module_path);
  CHECK(absolute_address >= memory_space.start);
  if (absolute_address > memory_space.end) return not_found_error;

  return std::make_pair(module_path, memory_space.start);
}