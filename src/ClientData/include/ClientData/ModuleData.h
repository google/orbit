// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_MODULE_DATA_H_
#define CLIENT_DATA_MODULE_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include <cinttypes>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/synchronization/mutex.h"

namespace orbit_client_data {

// Represents information about a module on the client. This class if fully synchronized.
class ModuleData final {
 public:
  explicit ModuleData(orbit_grpc_protos::ModuleInfo module_info);

  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] const std::string& file_path() const;
  [[nodiscard]] uint64_t file_size() const;
  [[nodiscard]] const std::string& build_id() const;
  [[nodiscard]] uint64_t load_bias() const;
  [[nodiscard]] uint64_t executable_segment_offset() const;
  [[nodiscard]] orbit_grpc_protos::ModuleInfo::ObjectFileType object_file_type() const;
  [[nodiscard]] std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment> GetObjectSegments() const;

  [[nodiscard]] orbit_symbol_provider::ModuleIdentifier module_id() const;

  [[nodiscard]] uint64_t ConvertFromVirtualAddressToOffsetInFile(uint64_t virtual_address) const;
  [[nodiscard]] uint64_t ConvertFromOffsetInFileToVirtualAddress(uint64_t offset_in_file) const;

  // Returns true if the module was unloaded (symbols were removed) and false otherwise.
  [[nodiscard]] bool UpdateIfChangedAndUnload(orbit_grpc_protos::ModuleInfo new_module_info);
  // This method does not update the module in case symbols are already loaded, even if the module
  // would need to be updated. Returns true if the update was successful or no update was needed,
  // and false if the module cannot be updated because symbols are already loaded.
  [[nodiscard]] bool UpdateIfChangedAndNotLoaded(orbit_grpc_protos::ModuleInfo new_module_info);

  [[nodiscard]] const FunctionInfo* FindFunctionByVirtualAddress(uint64_t virtual_address,
                                                                 bool is_exact) const;
  [[nodiscard]] const FunctionInfo* FindFunctionFromHash(uint64_t hash) const;
  [[nodiscard]] const FunctionInfo* FindFunctionFromPrettyName(std::string_view pretty_name) const;
  [[nodiscard]] std::vector<const FunctionInfo*> GetFunctions() const;

  enum class SymbolCompleteness {
    kNoSymbols = 0,
    kDynamicLinkingAndUnwindInfo = 1,  // i.e., "fallback symbols"
    kDebugSymbols = 2,
  };
  [[nodiscard]] SymbolCompleteness GetLoadedSymbolsCompleteness() const;
  [[nodiscard]] bool AreDebugSymbolsLoaded() const;
  [[nodiscard]] bool AreAtLeastFallbackSymbolsLoaded() const;

  void AddSymbols(const orbit_grpc_protos::ModuleSymbols& module_symbols);
  void AddFallbackSymbols(const orbit_grpc_protos::ModuleSymbols& module_symbols);

 private:
  [[nodiscard]] bool NeedsUpdate(const orbit_grpc_protos::ModuleInfo& new_module_info) const;

  void AddSymbolsInternal(const orbit_grpc_protos::ModuleSymbols& module_symbols,
                          SymbolCompleteness completeness);

  mutable absl::Mutex mutex_;
  orbit_grpc_protos::ModuleInfo module_info_ ABSL_GUARDED_BY(mutex_);

  SymbolCompleteness loaded_symbols_completeness_ ABSL_GUARDED_BY(mutex_) =
      SymbolCompleteness::kNoSymbols;
  std::map<uint64_t, std::unique_ptr<FunctionInfo>> functions_ ABSL_GUARDED_BY(mutex_);
  absl::flat_hash_map<std::string_view, FunctionInfo*> name_to_function_info_map_
      ABSL_GUARDED_BY(mutex_);

  // TODO(b/168799822): This is a map of hash to function used for preset loading. Currently,
  // presets are based on a hash of the functions pretty name. This should be changed to not use
  // hashes anymore.
  absl::flat_hash_map<uint64_t, FunctionInfo*> hash_to_function_map_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_MODULE_DATA_H_
