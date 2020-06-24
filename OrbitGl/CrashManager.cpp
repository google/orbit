// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CrashManager.h"

#include "OrbitBase/Logging.h"
#include "services.grpc.pb.h"

namespace {

class CrashManagerImpl final : public CrashManager {
 public:
  explicit CrashManagerImpl(std::shared_ptr<grpc::Channel> channel);

  void CrashOrbitService(GetCrashRequest_CrashType crash_type) override;

 private:
  std::unique_ptr<CrashService::Stub> crash_service_;
};

CrashManagerImpl::CrashManagerImpl(std::shared_ptr<grpc::Channel> channel)
    : crash_service_(CrashService::NewStub(channel)) {}

void CrashManagerImpl::CrashOrbitService(GetCrashRequest_CrashType crash_type) {
  GetCrashRequest request;
  request.set_crash_type(crash_type);

  GetCrashResponse response;
  grpc::ClientContext context;

  grpc::Status status =
      crash_service_->CrashOrbitService(&context, request, &response);
  if (!status.ok()) {
    ERROR("Grpc call failed: %s", status.error_message());
  }
}

}  // namespace

std::unique_ptr<CrashManager> CrashManager::Create(
    std::shared_ptr<grpc::Channel> channel) {
  std::unique_ptr<CrashManagerImpl> impl =
      std::make_unique<CrashManagerImpl>(channel);
  return impl;
}
