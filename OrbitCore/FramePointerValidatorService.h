// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_
#define ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_

#include <vector>

#include "OrbitFunction.h"
#include "ProcessUtils.h"
#include "TransactionService.h"

// Runs on the service and receives requests (Msg_ValidateFramePointers) from
// FramePointerValidatorClient to validate whether certain modules are
// compiled with frame pointers.
// It returns a list of functions that don't have a prologue and epilogue
// associated with frame pointers (see FunctionFramePointerValidator).
class FramePointerValidatorService {
 public:
  FramePointerValidatorService(const ProcessList* process_list,
                               TransactionService* transaction_service);
  FramePointerValidatorService() = delete;

  FramePointerValidatorService(const FramePointerValidatorService&) = delete;
  FramePointerValidatorService& operator=(const FramePointerValidatorService&) =
      delete;
  FramePointerValidatorService(FramePointerValidatorService&&) = delete;
  FramePointerValidatorService& operator=(FramePointerValidatorService&&) =
      delete;

 private:
  void HandleRequest(const Message& message);

  const ProcessList* process_list_;
  TransactionService* transaction_service_;
};

#endif  // ORBIT_CORE_FRAME_POINTER_VALIDATOR_SERVICE_H_
