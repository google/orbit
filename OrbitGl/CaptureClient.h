// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_CLIENT_H_
#define ORBIT_GL_CAPTURE_CLIENT_H_

#include "grpcpp/channel.h"
#include "services.grpc.pb.h"

class CaptureClient {
 public:
  explicit CaptureClient(const std::shared_ptr<grpc::Channel>& channel)
      : capture_service_{CaptureService::NewStub(channel)} {}

  void Capture();
  void StopCapture();

 private:
  void FinishCapture();

  std::unique_ptr<CaptureService::Stub> capture_service_;
  std::unique_ptr<grpc::ClientReaderWriter<CaptureRequest, CaptureEvent>>
      reader_writer_;
};

#endif  // ORBIT_GL_CAPTURE_CLIENT_H_
