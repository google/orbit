// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURE_SERVICE_UTILS_H_
#define CAPTURE_SERVICE_CAPTURE_SERVICE_UTILS_H_

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_service {

// Provide utilities that depend on gPRC and can be shared by the platform-specific native orbit
// capture services.
orbit_grpc_protos::CaptureRequest WaitForStartCaptureRequestFromClient(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer);
void WaitForStopCaptureRequestFromClient(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer);

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_SERVICE_UTILS_H_
