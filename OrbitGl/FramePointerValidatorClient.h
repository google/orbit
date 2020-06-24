// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FRAME_POINTER_VALIDATOR_CLIENT_H_
#define ORBIT_CORE_FRAME_POINTER_VALIDATOR_CLIENT_H_

#include "OrbitModule.h"
#include "TransactionClient.h"
#include "grpcpp/grpcpp.h"
#include "services.grpc.pb.h"

class OrbitApp;

// This class can be called from the UI on the client in order to validate
// whether certain modules are compiled with frame pointers.
// It will send a request to FramePointerValidatorServiceImpl, to perform the
// analysis on the client.
// On a response, it will display the number of functions that have a non-valid
// prologue/epilogue as an infobox.
// TODO(kuebler): The right output format need to be discussed and decided.
class FramePointerValidatorClient {
 public:
  explicit FramePointerValidatorClient(OrbitApp* core_app,
                                       std::shared_ptr<grpc::Channel> channel);

  FramePointerValidatorClient() = delete;
  FramePointerValidatorClient(const FramePointerValidatorClient&) = delete;
  FramePointerValidatorClient& operator=(const FramePointerValidatorClient&) =
      delete;
  FramePointerValidatorClient(FramePointerValidatorClient&&) = delete;
  FramePointerValidatorClient& operator=(FramePointerValidatorClient&&) =
      delete;

  void AnalyzeModules(const std::vector<std::shared_ptr<Module>>& modules);

 private:
  OrbitApp* app_;
  std::unique_ptr<FramePointerValidatorService::Stub>
      frame_pointer_validator_service_;
};

#endif  // ORBIT_CORE_FRAME_POINTER_VALIDATOR_CLIENT_H_
