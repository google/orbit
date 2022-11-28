// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_INSTRUMENT_PROCESS_H_
#define USER_SPACE_INSTRUMENTATION_INSTRUMENT_PROCESS_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"
#include "UserSpaceInstrumentation/AddressRange.h"

namespace orbit_user_space_instrumentation {

class InstrumentedProcess;

// `InstrumentationManager` is a globally unique object containing the bookkeeping for all user
// space instrumentation (in the `process_map_` member). Its lifetime is pretty much identical to
// the lifetime of the profiling service.
class InstrumentationManager {
 public:
  InstrumentationManager(const InstrumentationManager&) = delete;
  InstrumentationManager(InstrumentationManager&&) = delete;
  InstrumentationManager& operator=(const InstrumentationManager&) = delete;
  InstrumentationManager& operator=(InstrumentationManager&&) = delete;
  ~InstrumentationManager();

  [[nodiscard]] static std::unique_ptr<InstrumentationManager> Create();

  struct InstrumentationResult {
    absl::flat_hash_set<uint64_t> instrumented_function_ids;
    absl::flat_hash_map<uint64_t, std::string> function_ids_to_error_messages;
    std::vector<AddressRange> entry_trampoline_address_ranges;
    AddressRange return_trampoline_address_range;
    std::filesystem::path injected_library_path;
  };

  // On the first call to this function we inject OrbitUserSpaceInstrumentation.so into the target
  // process and create the return trampoline. On each call we create trampolines for functions that
  // were not instrumented before and instrument all functions by overwriting the prologue with a
  // jump into the trampoline. Returns the function_id's of the instrumented functions and -
  // potentially - error messages for functions where the instrumentation failed. Note that there is
  // no guarantee that we can instrument all the functions in a binary. It also returns the address
  // ranges dedicated to trampolines, including the return trampoline, and the map name of the
  // injected library.
  [[nodiscard]] ErrorMessageOr<InstrumentationResult> InstrumentProcess(
      const orbit_grpc_protos::CaptureOptions& capture_options);

  // Undo the instrumentation of the functions. Leaves the library and trampolines in the target
  // process intact. We merely restore the function prologues that were overwritten.
  [[nodiscard]] ErrorMessageOr<void> UninstrumentProcess(pid_t pid);

 private:
  InstrumentationManager() = default;

  absl::flat_hash_map<pid_t, std::unique_ptr<InstrumentedProcess>> process_map_;
};

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_INSTRUMENT_PROCESS_H_