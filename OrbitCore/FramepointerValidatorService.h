// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FRAMEPOINTER_VALIDATOR_SERVICE_H_
#define ORBIT_CORE_FRAMEPOINTER_VALIDATOR_SERVICE_H_

#include <vector>

#include "OrbitFunction.h"
#include "ProcessUtils.h"
#include "TransactionService.h"

// Runs on the service and receives requests (Msg_ValidateFramepointer) from
// FramepointerValidatorClient to validate whether certain modules are
// compiled with framepointers.
// It responds a list of all functions that are identified, to don't have
// a valid prologue/epilogue (see FunctionFramepointerValidator).
class FramepointerValidatorService {
 public:
  FramepointerValidatorService(const ProcessList* process_list,
                               TransactionService* transaction_service);
  FramepointerValidatorService() = delete;

  FramepointerValidatorService(const FramepointerValidatorService&) = delete;
  FramepointerValidatorService& operator=(const FramepointerValidatorService&) =
      delete;
  FramepointerValidatorService(FramepointerValidatorService&&) = delete;
  FramepointerValidatorService& operator=(FramepointerValidatorService&&) =
      delete;

 private:
  void HandleRequest(const Message& message);

  static std::vector<std::shared_ptr<Function>> CheckFramepointers(
      Pdb* pdb, bool is64Bit);

  const ProcessList* process_list_;
  TransactionService* transaction_service_;
};

#endif  // ORBIT_CORE_FRAMEPOINTER_VALIDATOR_SERVICE_H_
