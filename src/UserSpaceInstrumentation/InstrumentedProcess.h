// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_INSTRUMENTED_PROCESS_H_
#define USER_SPACE_INSTRUMENTATION_INSTRUMENTED_PROCESS_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <vector>

#include "AddressRange.h"
#include "OrbitBase/Result.h"
#include "capture.pb.h"

namespace orbit_user_space_instrumentation {

class InstrumentedProcess {
 public:
  ErrorMessageOr<void> Init(const orbit_grpc_protos::CaptureOptions& capture_options);

  // Instruments the functions capture_options.instrumented_functions and returns a set of
  // function_id's
  ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentFunctions(
      const orbit_grpc_protos::CaptureOptions& capture_options);

  ErrorMessageOr<void> UninstrumentFunctions(
      const orbit_grpc_protos::CaptureOptions& capture_options);

 private:
  ErrorMessageOr<uint64_t> GetTrampolineMemory(AddressRange address_range);
  ErrorMessageOr<void> ReleaseMostRecentTrampolineMemory(AddressRange address_range);

  ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> ModulesFromModulePath(
      std::string path);

  pid_t pid_ = -1;

  uint64_t entry_payload_function_address_ = 0;
  uint64_t exit_payload_function_address_ = 0;
  uint64_t return_trampoline_address_ = 0;

  absl::flat_hash_map<uint64_t, uint64_t> relocation_map_;

  struct TrampolineData {
    uint64_t trampoline_address;
    uint64_t address_after_prologue;
    std::vector<uint8_t> function_data;
  };
  // Maps function addresses to TrampolineData.
  absl::flat_hash_map<uint64_t, TrampolineData> trampoline_map_;

  // Trampolines are allocated in chunks of kTrampolinesPerChunk. Trampolines are fixed size
  // (compare `GetMaxTrampolineSize`) and are never freed; we just allocate new chunks when that
  // last one is filled up. Each module gets it own sequence of chunks. (TODO:identify module by
  // address range, not by file path)
  // (`trampolines_for_modules_`).
  static constexpr int kTrampolinesPerChunk = 1000;
  struct TrampolineMemoryChunk {
    uint64_t address = 0;
    int first_available = 0;
  };
  typedef std::vector<TrampolineMemoryChunk> TrampolineMemoryChunks;
  absl::flat_hash_map<AddressRange, TrampolineMemoryChunks> trampolines_for_modules_;

  // Map path of a module in a process to all loaded instances of that module (usually this will
  // only be one but a module can be loaded more than once).
  absl::flat_hash_map<std::string, std::vector<orbit_grpc_protos::ModuleInfo>> modules_from_path_;
};

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_INSTRUMENTED_PROCESS_H_