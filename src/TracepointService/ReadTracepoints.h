// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TRACEPOINT_SERVICE_READ_TRACEPOINTS_H_
#define TRACEPOINT_SERVICE_READ_TRACEPOINTS_H_

#include <vector>

#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_tracepoint_service {

ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> ReadTracepoints();

}  // namespace orbit_tracepoint_service

#endif  // TRACEPOINT_SERVICE_READ_TRACEPOINTS_H_
