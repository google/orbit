// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_
#define CAPTURE_SERVICE_BASE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_

#include <grpcpp/grpcpp.h>

#include "CaptureServiceBase/ClientCaptureEventCollectorManager.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_service_base {

// Create a `ClientCaptureEventCollectorManager` which builds and managers a
// `GrpcCaptureEventCollector` for native orbit capture services.
[[nodiscard]] std::unique_ptr<ClientCaptureEventCollectorManager>
CreateGrpcClientCaptureEventCollectorManager(
    grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse, orbit_grpc_protos::CaptureRequest>*
        reader_writer);

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_