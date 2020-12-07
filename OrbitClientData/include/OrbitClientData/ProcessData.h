// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_PROCESS_DATA_H_
#define ORBIT_CLIENT_DATA_PROCESS_DATA_H_

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <inttypes.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitClientData/ModuleData.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "module.pb.h"
#include "process.pb.h"
#include "symbol.pb.h"

// Small struct to model a space in memory occupied by a module.
struct MemorySpace {
  explicit MemorySpace(uint64_t start, uint64_t end) : start(start), end(end) {}
  uint64_t start;
  uint64_t end;
  [[nodiscard]] std::string FormattedAddressRange() const {
    return absl::StrFormat("[%016" PRIx64 " - %016" PRIx64 "]", start, end);
  }
};

// Contains current information about process
class ProcessData final {
 public:
  ProcessData();
  ProcessData(const ProcessData&) = default;
  ProcessData& operator=(const ProcessData&) = default;
  ProcessData(ProcessData&&) = default;
  ProcessData& operator=(ProcessData&&) = default;

  explicit ProcessData(orbit_grpc_protos::ProcessInfo process_info)
      : process_info_(std::move(process_info)) {}

  void SetProcessInfo(const orbit_grpc_protos::ProcessInfo& process_info) {
    process_info_ = process_info;
  }

  [[nodiscard]] int32_t pid() const { return process_info_.pid(); }
  [[nodiscard]] const std::string& name() const { return process_info_.name(); }
  [[nodiscard]] double cpu_usage() const { return process_info_.cpu_usage(); }
  [[nodiscard]] const std::string& full_path() const { return process_info_.full_path(); }
  [[nodiscard]] const std::string& command_line() const { return process_info_.command_line(); }
  [[nodiscard]] bool is_64_bit() const { return process_info_.is_64_bit(); }

  void UpdateModuleInfos(const std::vector<orbit_grpc_protos::ModuleInfo>& module_infos);

  [[nodiscard]] ErrorMessageOr<std::pair<std::string, uint64_t>> FindModuleByAddress(
      uint64_t absolute_address) const;

  uint64_t GetModuleBaseAddress(const std::string& module_path) const {
    CHECK(module_memory_map_.contains(module_path));
    return module_memory_map_.at(module_path).start;
  }
  const absl::flat_hash_map<std::string, MemorySpace>& GetMemoryMap() const {
    return module_memory_map_;
  }
  bool IsModuleLoaded(const std::string& module_path) const {
    return module_memory_map_.contains(module_path);
  }

 private:
  orbit_grpc_protos::ProcessInfo process_info_;

  // This is a map from module_path to the space in memory where that module is loaded
  absl::flat_hash_map<std::string, MemorySpace> module_memory_map_;
  std::map<uint64_t, const std::string> start_addresses_;
};

#endif  // ORBIT_GL_PROCESS_DATA_H_
