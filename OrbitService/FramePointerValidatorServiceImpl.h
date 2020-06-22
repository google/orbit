// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_
#define ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_

#include "services.grpc.pb.h"

// Runs on the service and receives requests (Msg_ValidateFramePointers) from
// FramePointerValidatorClient to validate whether certain modules are
// compiled with frame pointers.
// It returns a list of functions that don't have a prologue and epilogue
// associated with frame pointers (see FunctionFramePointerValidator).
class FramePointerValidatorServiceImpl final
    : public FramePointerValidatorService::Service {
 public:
  grpc::Status ValidateFramePointers(
      grpc::ServerContext* context, const ValidateFramePointersRequest* request,
      ValidateFramePointersResponse* response) override;

 private:
};

#endif  // ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_
