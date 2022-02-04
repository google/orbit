// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_H_
#define LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_H_

#include <grpcpp/grpcpp.h>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "LinuxCaptureService/LinuxCaptureServiceBase.h"

namespace orbit_linux_capture_service {

// Linux implementation of the grpc capture service.
class LinuxCaptureService final : public LinuxCaptureServiceBase,
                                  public orbit_grpc_protos::CaptureService::Service {
 public:
  grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer) override;
};

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_LINUX_CAPTURE_SERVICE_H_
