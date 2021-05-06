// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_PROCESS_DATA_H_
#define CLIENT_DATA_PROCESS_DATA_H_

#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/strings/str_format.h>
#include <inttypes.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "module.pb.h"
#include "process.pb.h"
#include "symbol.pb.h"

// Small struct to model a space in memory occupied by a module.
class ModuleInMemory {
 public:
  explicit ModuleInMemory(uint64_t start, uint64_t end, std::string file_path, std::string build_id)
      : start_(start), end_(end), file_path_{file_path}, build_id_{std::move(build_id)} {}
  [[nodiscard]] uint64_t start() const { return start_; }
  [[nodiscard]] uint64_t end() const { return end_; }
  [[nodiscard]] const std::string& file_path() const { return file_path_; }
  [[nodiscard]] const std::string& build_id() const { return build_id_; }
  [[nodiscard]] std::string FormattedAddressRange() const {
    return absl::StrFormat("[%016" PRIx64 " - %016" PRIx64 "]", start_, end_);
  }

 private:
  uint64_t start_;
  uint64_t end_;
  std::string file_path_;
  std::string build_id_;
};

// Contains current information about process
class ProcessData final {
 public:
  ProcessData();

  explicit ProcessData(orbit_grpc_protos::ProcessInfo process_info)
      : process_info_(std::move(process_info)) {}

  void SetProcessInfo(const orbit_grpc_protos::ProcessInfo& process_info);

  [[nodiscard]] int32_t pid() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] double cpu_usage() const;
  [[nodiscard]] const std::string& full_path() const;
  [[nodiscard]] const std::string& command_line() const;
  [[nodiscard]] bool is_64_bit() const;

  void UpdateModuleInfos(absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);
  void AddOrUpdateModuleInfo(const orbit_grpc_protos::ModuleInfo& module_infos);

  [[nodiscard]] ErrorMessageOr<ModuleInMemory> FindModuleByAddress(uint64_t absolute_address) const;

  std::optional<uint64_t> GetModuleBaseAddress(const std::string& module_path) const;
  absl::node_hash_map<std::string, ModuleInMemory> GetMemoryMapCopy() const;

  [[nodiscard]] std::optional<ModuleInMemory> FindModuleByPath(
      const std::string& module_path) const;

  [[nodiscard]] bool IsModuleLoaded(const std::string& module_path) const {
    return FindModuleByPath(module_path).has_value();
  }

 private:
  mutable absl::Mutex mutex_;
  orbit_grpc_protos::ProcessInfo process_info_;

  // This is a map from module_path to the space in memory where that module is loaded
  absl::node_hash_map<std::string, ModuleInMemory> module_memory_map_;
  std::map<uint64_t, std::string> start_addresses_;
};

#endif  // CLIENT_DATA_PROCESS_DATA_H_
