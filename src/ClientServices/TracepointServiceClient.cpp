// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/TracepointServiceClient.h"

#include <absl/strings/str_format.h>
#include <grpcpp/grpcpp.h>

#include <string>

#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_services {

using orbit_grpc_protos::GetTracepointListRequest;
using orbit_grpc_protos::GetTracepointListResponse;
using orbit_grpc_protos::TracepointInfo;
using orbit_grpc_protos::TracepointService;

TracepointServiceClient::TracepointServiceClient(const std::shared_ptr<grpc::Channel>& channel)
    : tracepoint_service_(TracepointService::NewStub(channel)) {}

ErrorMessageOr<std::vector<TracepointInfo>> TracepointServiceClient::GetTracepointList() const {
  GetTracepointListRequest request;
  GetTracepointListResponse response;

  std::unique_ptr<grpc::ClientContext> context = std::make_unique<grpc::ClientContext>();

  grpc::Status status = tracepoint_service_->GetTracepointList(context.get(), request, &response);
  if (!status.ok()) {
    const std::string& error_message =
        absl::StrFormat("gRPC call to GetTracepointList failed: %s (error_code=%d)",
                        status.error_message(), status.error_code());
    ORBIT_ERROR("gRPC call to GetTracepointList failed: %s (error_code=%d)", status.error_message(),
                status.error_code());
    return ErrorMessage(error_message);
  }

  const auto& tracepoints = response.tracepoints();

  return std::vector<TracepointInfo>{tracepoints.begin(), tracepoints.end()};
}

std::unique_ptr<TracepointServiceClient> TracepointServiceClient::Create(
    const std::shared_ptr<grpc::Channel>& channel) {
  return std::unique_ptr<TracepointServiceClient>(new TracepointServiceClient(channel));
}

}  // namespace orbit_client_services
