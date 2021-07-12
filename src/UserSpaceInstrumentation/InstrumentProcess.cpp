// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/InstrumentProcess.h"

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <dlfcn.h>
#include <unistd.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "InstrumentedProcess.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

absl::flat_hash_map<pid_t, InstrumentedProcess>& GetInstrumentedProcesses() {
  static absl::flat_hash_map<pid_t, InstrumentedProcess> instrumented_processes;
  return instrumented_processes;
}

}  // namespace

ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentProcess(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
  const pid_t pid = capture_options.pid();
  // If the user tries to instrument this instance of OrbitService we can't use user space
  // instrumentation: We need to attach to the target ...
  if (pid == getpid()) {
    return absl::flat_hash_set<uint64_t>();
  }

  absl::flat_hash_map<pid_t, InstrumentedProcess>& instrumented_processes =
      GetInstrumentedProcesses();
  if (!instrumented_processes.contains(pid)) {
    // TODO: Maybe delete entries belonging to processes that are not running anymore?
    InstrumentedProcess process;
    OUTCOME_TRY(process.Init(capture_options));
    instrumented_processes.emplace(pid, std::move(process));
  }
  OUTCOME_TRY(instrumented_function_ids,
              instrumented_processes[pid].InstrumentFunctions(capture_options));

  return std::move(instrumented_function_ids);
}

ErrorMessageOr<void> UninstrumentProcess(const orbit_grpc_protos::CaptureOptions& capture_options) {
  const pid_t pid = capture_options.pid();
  // If the user tries to instrument this instance of OrbitService we can't use user space
  // instrumentation: We need to attach to the target ...
  if (pid == getpid()) {
    return outcome::success();
  }

  absl::flat_hash_map<pid_t, InstrumentedProcess>& instrumented_processes =
      GetInstrumentedProcesses();
  OUTCOME_TRY(instrumented_processes[pid].UninstrumentFunctions(capture_options));
  // TODO: Also switch off emitting events to Orbit.

  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation
