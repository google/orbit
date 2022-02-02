// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_
#define CAPTURE_SERVICE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_

#include <grpcpp/grpcpp.h>

#include "CaptureService/ClientCaptureEventCollectorBuilder.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_service {

// Create a `ClientCaptureEventCollectorBuilder` which builds a `GrpcCaptureEventCollector` for
// native orbit capture services.
std::unique_ptr<ClientCaptureEventCollectorBuilder> CreateGrpcClientCaptureEventCollectorBuilder(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer);

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_