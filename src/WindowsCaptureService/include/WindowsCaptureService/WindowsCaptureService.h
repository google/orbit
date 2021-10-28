// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_CAPTURE_SERVICE_WINDOWS_CAPTURE_SERVICE_H_
#define WINDOWS_CAPTURE_SERVICE_WINDOWS_CAPTURE_SERVICE_H_

#include "CaptureService/CaptureService.h"

namespace orbit_windows_capture_service {

// Windows implementation of the grpc capture service.
class WindowsCaptureService final : public orbit_capture_service::CaptureService {
 public:
  grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer) override;
};

}  // namespace orbit_windows_capture_service

#endif  // WINDOWS_CAPTURE_SERVICE_WINDOWS_CAPTURE_SERVICE_H_
