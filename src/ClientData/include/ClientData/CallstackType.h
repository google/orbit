// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_TYPES_H_
#define CLIENT_DATA_CALLSTACK_TYPES_H_

#include <cstdint>

#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

using ThreadID = uint32_t;

// This enum represents the different (error) types of a callstack. `kComplete` represents and
// intact callstack, while the other enum values describe different errors. In addition to the
// values in `orbit_grpc_protos::Callstack`, error types detected by the client are added.
// The enum must be kept in sync with the one in `orbit_grpc_protos::Callstack`.
enum class CallstackType {
  kComplete = 0,
  kDwarfUnwindingError = 1,
  kFramePointerUnwindingError = 2,
  kInUprobes = 3,
  kInUserSpaceInstrumentation = 7,
  kCallstackPatchingFailed = 4,
  kStackTopForDwarfUnwindingTooSmall = 5,
  kStackTopDwarfUnwindingError = 6,

  // These are set by the client and are in addition to the ones in
  // Callstack::CallstackType.
  kFilteredByMajorityOutermostFrame = 100
};

[[nodiscard]] std::string CallstackTypeToString(CallstackType callstack_type);

[[nodiscard]] std::string CallstackTypeToDescription(CallstackType callstack_type);

[[nodiscard]] CallstackType GrpcCallstackTypeToCallstackType(
    orbit_grpc_protos::Callstack::CallstackType callstack_type);

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_TYPES_H_
