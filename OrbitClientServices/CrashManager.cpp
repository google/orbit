// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientServices/CrashManager.h"

#include "OrbitBase/Logging.h"
#include "services.grpc.pb.h"

namespace {

class CrashManagerImpl final : public CrashManager {
 public:
  explicit CrashManagerImpl(std::shared_ptr<grpc::Channel> channel);

  void CrashOrbitService(
      CrashOrbitServiceRequest_CrashType crash_type) override;

 private:
  std::unique_ptr<CrashService::Stub> crash_service_;
};

CrashManagerImpl::CrashManagerImpl(std::shared_ptr<grpc::Channel> channel)
    : crash_service_(CrashService::NewStub(channel)) {}

void CrashManagerImpl::CrashOrbitService(
    CrashOrbitServiceRequest_CrashType crash_type) {
  CrashOrbitServiceRequest request;
  request.set_crash_type(crash_type);

  CrashOrbitServiceResponse response;
  grpc::ClientContext context;

  std::function<void(grpc::Status)> callback = [](::grpc::Status status) {
    if (!status.ok()) {
      ERROR("Grpc call failed: %s", status.error_message());
    }
  };
  crash_service_->experimental_async()->CrashOrbitService(&context, &request,
                                                          &response, callback);
}

}  // namespace

std::unique_ptr<CrashManager> CrashManager::Create(
    std::shared_ptr<grpc::Channel> channel) {
  std::unique_ptr<CrashManagerImpl> impl =
      std::make_unique<CrashManagerImpl>(channel);
  return impl;
}
