// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_CAPTURE_SERVICE_IMPL_H_
#define ORBIT_SERVICE_CAPTURE_SERVICE_IMPL_H_

#include "services.grpc.pb.h"

namespace orbit_service {

class CaptureServiceImpl final
    : public orbit_grpc_protos::CaptureService::Service {
 public:
  grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>*
          reader_writer) override;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_CAPTURE_SERVICE_IMPL_H_
