// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_TRACEPOINT_SERVICE_CLIENT_H_
#define CLIENT_SERVICES_TRACEPOINT_SERVICE_CLIENT_H_

#include <grpcpp/channel.h>

#include <memory>
#include <vector>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {

class TracepointServiceClient {
 public:
  static std::unique_ptr<TracepointServiceClient> Create(
      const std::shared_ptr<grpc::Channel>& channel);

  [[nodiscard]] ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> GetTracepointList()
      const;

 private:
  explicit TracepointServiceClient(const std::shared_ptr<grpc::Channel>& channel);

  std::unique_ptr<orbit_grpc_protos::TracepointService::Stub> tracepoint_service_;
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_TRACEPOINT_SERVICE_CLIENT_H_
