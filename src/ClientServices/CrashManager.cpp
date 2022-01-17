// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/CrashManager.h"

#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <chrono>

#include "GrpcProtos/services.grpc.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_services {
namespace {

using orbit_grpc_protos::CrashOrbitServiceRequest;
using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType;
using orbit_grpc_protos::CrashOrbitServiceResponse;
using orbit_grpc_protos::CrashService;

constexpr uint64_t kTimeoutMilliseconds = 100;

class CrashManagerImpl final : public CrashManager {
 public:
  explicit CrashManagerImpl(const std::shared_ptr<grpc::Channel>& channel);

  void CrashOrbitService(CrashOrbitServiceRequest_CrashType crash_type) override;

 private:
  std::unique_ptr<CrashService::Stub> crash_service_;
};

CrashManagerImpl::CrashManagerImpl(const std::shared_ptr<grpc::Channel>& channel)
    : crash_service_(CrashService::NewStub(channel)) {}

void CrashManagerImpl::CrashOrbitService(CrashOrbitServiceRequest_CrashType crash_type) {
  CrashOrbitServiceRequest request;
  request.set_crash_type(crash_type);

  grpc::ClientContext context;
  std::chrono::system_clock::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(kTimeoutMilliseconds);
  context.set_deadline(deadline);

  CrashOrbitServiceResponse response;
  grpc::Status status = crash_service_->CrashOrbitService(&context, request, &response);

  if (status.error_code() != grpc::StatusCode::DEADLINE_EXCEEDED) {
    ORBIT_ERROR("CrashOrbitService returned code %i with error message %s", status.error_code(),
                status.error_message());
  }
}

}  // namespace

std::unique_ptr<CrashManager> CrashManager::Create(const std::shared_ptr<grpc::Channel>& channel) {
  std::unique_ptr<CrashManagerImpl> impl = std::make_unique<CrashManagerImpl>(channel);
  return impl;
}

}  // namespace orbit_client_services
