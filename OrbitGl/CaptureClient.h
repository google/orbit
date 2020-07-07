// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_CLIENT_H_
#define ORBIT_GL_CAPTURE_CLIENT_H_

#include <optional>

#include "CaptureListener.h"
#include "CaptureEventProcessor.h"
#include "OrbitBase/Logging.h"
#include "grpcpp/channel.h"
#include "services.grpc.pb.h"

class CaptureClient {
 public:
  explicit CaptureClient(const std::shared_ptr<grpc::Channel>& channel,
                         CaptureListener* capture_listener)
      : capture_service_{CaptureService::NewStub(channel)},
        capture_listener_{capture_listener} {
    CHECK(capture_listener_ != nullptr);
  }

  void Capture(
      int32_t pid,
      const std::vector<std::shared_ptr<Function>>& selected_functions);
  void StopCapture();

 private:
  void FinishCapture();

  std::unique_ptr<CaptureService::Stub> capture_service_;
  std::unique_ptr<grpc::ClientReaderWriter<CaptureRequest, CaptureResponse>>
      reader_writer_;

  CaptureListener* capture_listener_ = nullptr;

  std::optional<CaptureEventProcessor> event_processor_;
};

#endif  // ORBIT_GL_CAPTURE_CLIENT_H_
