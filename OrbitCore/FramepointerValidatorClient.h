// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FRAMEPOINTER_VALIDATOR_CLIENT_H_
#define ORBIT_CORE_FRAMEPOINTER_VALIDATOR_CLIENT_H_

#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "TransactionClient.h"

// This class can be called from the UI on the client in order to validate
// whether certain modules are compiled with framepointers.
// It will send a request to FramepointerValidatorService, to perform the
// analysis on the client.
// On a response, it will display the number of functions that have a non-valid
// prologue/epilogue as an infobox.
// TODO(kuebler): The right output format need to be discussed and decided.
class FramepointerValidatorClient {
 public:
  explicit FramepointerValidatorClient(TransactionClient* transaction_client);

  FramepointerValidatorClient() = delete;
  FramepointerValidatorClient(const FramepointerValidatorClient&) = delete;
  FramepointerValidatorClient& operator=(const FramepointerValidatorClient&) =
      delete;
  FramepointerValidatorClient(FramepointerValidatorClient&&) = delete;
  FramepointerValidatorClient& operator=(FramepointerValidatorClient&&) =
      delete;

  void AnalyzeModule(Process* process,
                     const std::vector<std::shared_ptr<Module>>& modules);

 private:
  void HandleResponse(const Message& message, uint64_t id);
  TransactionClient* transaction_client_;
  absl::flat_hash_map<uint64_t, std::vector<std::shared_ptr<Module>>>
      modules_map_;
  absl::Mutex id_mutex_;
};

#endif  // ORBIT_CORE_FRAMEPOINTER_VALIDATOR_CLIENT_H_
