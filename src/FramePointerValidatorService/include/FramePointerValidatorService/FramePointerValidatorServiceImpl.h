// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FRAME_POINTER_VALIDATOR_SERVICE_FRAME_POINTER_VALIDATOR_SERVICE_IMPL_H_
#define FRAME_POINTER_VALIDATOR_SERVICE_FRAME_POINTER_VALIDATOR_SERVICE_IMPL_H_

#include <grpcpp/grpcpp.h>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_frame_pointer_validator_service {

// Runs on the service and receives requests from FramePointerValidatorClient to
// validate whether certain modules are compiled with frame pointers.
// It returns a list of functions that don't have a prologue and epilogue
// associated with frame pointers (see FunctionFramePointerValidator).
class FramePointerValidatorServiceImpl final
    : public orbit_grpc_protos::FramePointerValidatorService::Service {
 public:
  [[nodiscard]] grpc::Status ValidateFramePointers(
      grpc::ServerContext* context, const orbit_grpc_protos::ValidateFramePointersRequest* request,
      orbit_grpc_protos::ValidateFramePointersResponse* response) override;
};

}  // namespace orbit_frame_pointer_validator_service

#endif  // FRAME_POINTER_VALIDATOR_SERVICE_FRAME_POINTER_VALIDATOR_SERVICE_IMPL_H_
