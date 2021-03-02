// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GRPC_PROTOS_CONSTANTS_H_
#define GRPC_PROTOS_CONSTANTS_H_

#include <stdint.h>

namespace orbit_grpc_protos {
constexpr uint64_t kInvalidInternId = 0;
constexpr uint64_t kInvalidFunctionId = 0;

constexpr int kMissingInfo = -1;

// Reserved producer IDs.
constexpr uint64_t kLinuxTracingProducerId = 0;
constexpr uint64_t kMemoryInfoProducerId = 1;
constexpr uint64_t kExternalProducerStartingId = 1024;
}  // namespace orbit_grpc_protos

#endif  // GRPC_PROTOS_CONSTANTS_H_
