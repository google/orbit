// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_IMPL_H_
#define ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_IMPL_H_

#include "services_ggp.grpc.pb.h"

// Logic and data behind the server's behavior.
class CaptureClientGgpServiceImpl final
    : public orbit_grpc_protos::CaptureClientGgpService::Service {
 public:
  grpc::Status SayHello(grpc::ServerContext*, const orbit_grpc_protos::HelloRequest* request,
                        orbit_grpc_protos::HelloReply* reply) override;
};

#endif  // ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_IMPL_H_
