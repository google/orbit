// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_INSTRUMENT_PROCESS_H_
#define USER_SPACE_INSTRUMENTATION_INSTRUMENT_PROCESS_H_

#include <absl/container/flat_hash_set.h>

#include "OrbitBase/Result.h"
#include "capture.pb.h"

namespace orbit_user_space_instrumentation {

// On the first call to this function the we inject UserSpaceInstrumentationLib into the target
// process and create the return trampoline.
// On each call we create trampolines for functions that were not instrumented before and instrument
// all functions given in `capture_options` functions.
[[nodiscard]] ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentProcess(
    const orbit_grpc_protos::CaptureOptions& capture_options);

// Undo the instrumentation of the functions. Leaves the library and trampolines in the target
// process intact.
[[nodiscard]] ErrorMessageOr<void> UninstrumentProcess(
    const orbit_grpc_protos::CaptureOptions& capture_options);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_INSTRUMENT_PROCESS_H_