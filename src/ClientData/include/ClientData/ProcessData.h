// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_PROCESS_DATA_H_
#define CLIENT_DATA_PROCESS_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <inttypes.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_client_data {

// Small struct to model a space in memory occupied by a module.
class ModuleInMemory {
 public:
  explicit ModuleInMemory(uint64_t start, uint64_t end, std::string file_path, std::string build_id)
      : start_(start),
        end_(end),
        file_path_{std::move(file_path)},
        build_id_{std::move(build_id)} {}

  [[nodiscard]] uint64_t start() const { return start_; }
  [[nodiscard]] uint64_t end() const { return end_; }
  [[nodiscard]] const std::string& file_path() const { return file_path_; }
  [[nodiscard]] const std::string& build_id() const { return build_id_; }
  [[nodiscard]] orbit_symbol_provider::ModuleIdentifier module_id() const {
    return orbit_symbol_provider::ModuleIdentifier{file_path(), build_id()};
  }
  [[nodiscard]] std::string FormattedAddressRange() const {
    return absl::StrFormat("[%016x - %016x]", start_, end_);
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

  [[nodiscard]] uint32_t pid() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] double cpu_usage() const;
  [[nodiscard]] const std::string& full_path() const;
  [[nodiscard]] const std::string& command_line() const;
  [[nodiscard]] bool is_64_bit() const;
  [[nodiscard]] const std::string& build_id() const;

  void UpdateModuleInfos(absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);

  // This method removes all modules with addresses intersecting with module_info.
  // Some background on this. The service currently does not send any unmap event
  // to the client, which means that if the client receives a module with mapping
  // intersecting with exiting mapping the old module was likely unloaded.
  void AddOrUpdateModuleInfo(const orbit_grpc_protos::ModuleInfo& module_info);

  [[nodiscard]] ErrorMessageOr<ModuleInMemory> FindModuleByAddress(uint64_t absolute_address) const;

  // Returns module base addresses. Note that the same module could be mapped twice in which case
  // this function returns two base addresses. If no module found the function returns empty vector.
  [[nodiscard]] std::vector<uint64_t> GetModuleBaseAddresses(std::string_view module_path,
                                                             std::string_view build_id) const;

  [[nodiscard]] std::map<uint64_t, ModuleInMemory> GetMemoryMapCopy() const;
  [[nodiscard]] std::vector<orbit_symbol_provider::ModuleIdentifier> GetUniqueModuleIdentifiers()
      const;

  [[nodiscard]] std::vector<std::string> FindModuleBuildIdsByPath(
      std::string_view module_path) const;

  // Returns the list of modules with the given filename. Note this method matches based on the
  // actual filename and not based on the full path.
  [[nodiscard]] std::vector<ModuleInMemory> FindModulesByFilename(std::string_view filename) const;

  [[nodiscard]] bool IsModuleLoadedByProcess(std::string_view module_path) const {
    return !FindModuleBuildIdsByPath(module_path).empty();
  }

  [[nodiscard]] bool IsModuleLoadedByProcess(const ModuleData* module) const;

 private:
  mutable absl::Mutex mutex_;
  orbit_grpc_protos::ProcessInfo process_info_ ABSL_GUARDED_BY(mutex_);
  std::map<uint64_t, ModuleInMemory> start_address_to_module_in_memory_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_PROCESS_DATA_H_
