// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_TYPES_H_
#define CLIENT_DATA_CALLSTACK_TYPES_H_

#include <cstdint>
#include <string>

#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

using ThreadID = uint32_t;

// This enum represents the different (error) types of a callstack. `kComplete` represents an
// intact callstack, while the other enum values describe different errors. In addition to the
// values in `orbit_grpc_protos::Callstack`, error types detected by the client are added.
// The enum must be kept in sync with the one in `orbit_grpc_protos::Callstack`.
enum class CallstackType {
  kComplete,
  kDwarfUnwindingError,
  kFramePointerUnwindingError,
  kInUprobes,
  kInUserSpaceInstrumentation,
  kCallstackPatchingFailed,
  kStackTopForDwarfUnwindingTooSmall,
  kStackTopDwarfUnwindingError,

  // These are set by the client and are in addition to the ones in
  // orbit_grpc_protos::Callstack::CallstackType.
  kFilteredByMajorityOutermostFrame
};

[[nodiscard]] std::string CallstackTypeToString(CallstackType callstack_type);

[[nodiscard]] std::string CallstackTypeToDescription(CallstackType callstack_type);

[[nodiscard]] CallstackType GrpcCallstackTypeToCallstackType(
    orbit_grpc_protos::Callstack::CallstackType callstack_type);

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_TYPES_H_
