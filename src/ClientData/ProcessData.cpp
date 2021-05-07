// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ProcessData.h"

#include <absl/container/flat_hash_map.h>

#include <cstdint>
#include <vector>

#include "OrbitBase/Result.h"
#include "absl/strings/str_format.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleInfo;

namespace orbit_client_data {

ProcessData::ProcessData() { process_info_.set_pid(-1); }

void ProcessData::SetProcessInfo(const orbit_grpc_protos::ProcessInfo& process_info) {
  absl::MutexLock lock(&mutex_);
  process_info_ = process_info;
}

int32_t ProcessData::pid() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.pid();
}

const std::string& ProcessData::name() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.name();
}

double ProcessData::cpu_usage() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.cpu_usage();
}

const std::string& ProcessData::full_path() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.full_path();
}

const std::string& ProcessData::command_line() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.command_line();
}

bool ProcessData::is_64_bit() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.is_64_bit();
}

const std::string& ProcessData::build_id() const {
  absl::MutexLock lock(&mutex_);
  return process_info_.build_id();
}

void ProcessData::UpdateModuleInfos(absl::Span<const ModuleInfo> module_infos) {
  absl::MutexLock lock(&mutex_);
  module_memory_map_.clear();
  start_addresses_.clear();

  for (const auto& module_info : module_infos) {
    {
      const auto [it, success] = module_memory_map_.try_emplace(
          module_info.file_path(),
          ModuleInMemory{module_info.address_start(), module_info.address_end(),
                         module_info.file_path(), module_info.build_id()});
      CHECK(success);
    }
    {
      const auto [it, success] =
          start_addresses_.try_emplace(module_info.address_start(), module_info.file_path());
      CHECK(success);
    }
  }
}

std::optional<ModuleInMemory> ProcessData::FindModuleByPath(const std::string& module_path) const {
  absl::MutexLock lock(&mutex_);
  auto it = module_memory_map_.find(module_path);
  if (it == module_memory_map_.end()) {
    return std::nullopt;
  }

  return it->second;
}

void ProcessData::AddOrUpdateModuleInfo(const ModuleInfo& module_info) {
  absl::MutexLock lock(&mutex_);
  module_memory_map_.insert_or_assign(
      module_info.file_path(),
      ModuleInMemory{module_info.address_start(), module_info.address_end(),
                     module_info.file_path(), module_info.build_id()});
  start_addresses_.insert_or_assign(module_info.address_start(), module_info.file_path());
}

ErrorMessageOr<ModuleInMemory> ProcessData::FindModuleByAddress(uint64_t absolute_address) const {
  absl::MutexLock lock(&mutex_);
  if (start_addresses_.empty()) {
    return ErrorMessage(absl::StrFormat("Unable to find module for address %016" PRIx64
                                        ": No modules loaded by process %s",
                                        absolute_address, process_info_.name()));
  }

  ErrorMessage not_found_error =
      ErrorMessage(absl::StrFormat("Unable to find module for address %016" PRIx64
                                   ": No module loaded at this address by process %s",
                                   absolute_address, process_info_.name()));

  auto it = start_addresses_.upper_bound(absolute_address);
  if (it == start_addresses_.begin()) return not_found_error;

  --it;
  const std::string& module_path = it->second;
  const ModuleInMemory& module_in_memory = module_memory_map_.at(module_path);
  CHECK(absolute_address >= module_in_memory.start());
  if (absolute_address > module_in_memory.end()) return not_found_error;

  return module_in_memory;
}

std::optional<uint64_t> ProcessData::GetModuleBaseAddress(const std::string& module_path) const {
  absl::MutexLock lock(&mutex_);
  if (!module_memory_map_.contains(module_path)) {
    return std::nullopt;
  }
  return module_memory_map_.at(module_path).start();
}

absl::node_hash_map<std::string, ModuleInMemory> ProcessData::GetMemoryMapCopy() const {
  absl::MutexLock lock(&mutex_);
  return module_memory_map_;
}

}  // namespace orbit_client_data
