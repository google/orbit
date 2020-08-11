// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_
#define ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_

#include "services.grpc.pb.h"

namespace orbit_service {

// Runs on the service and receives requests from FramePointerValidatorClient to
// validate whether certain modules are compiled with frame pointers.
// It returns a list of functions that don't have a prologue and epilogue
// associated with frame pointers (see FunctionFramePointerValidator).
class FramePointerValidatorServiceImpl final
    : public orbit_grpc_protos::FramePointerValidatorService::Service {
 public:
  [[nodiscard]] grpc::Status ValidateFramePointers(
      grpc::ServerContext* context,
      const orbit_grpc_protos::ValidateFramePointersRequest* request,
      orbit_grpc_protos::ValidateFramePointersResponse* response) override;
};

}  // namespace orbit_service

#endif  // ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_
