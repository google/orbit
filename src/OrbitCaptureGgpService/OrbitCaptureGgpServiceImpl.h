// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_IMPL_H_
#define ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_IMPL_H_

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>

#include "GrpcProtos/services_ggp.grpc.pb.h"
#include "GrpcProtos/services_ggp.pb.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitClientGgp/ClientGgp.h"

// Logic and data behind the server's behavior.
class CaptureClientGgpServiceImpl final
    : public orbit_grpc_protos::CaptureClientGgpService::Service {
 public:
  CaptureClientGgpServiceImpl();

  [[nodiscard]] grpc::Status StartCapture(
      grpc::ServerContext* context, const orbit_grpc_protos::StartCaptureRequest* request,
      orbit_grpc_protos::StartCaptureResponse* response) override;

  [[nodiscard]] grpc::Status StopCapture(grpc::ServerContext* context,
                                         const orbit_grpc_protos::StopCaptureRequest* request,
                                         orbit_grpc_protos::StopCaptureResponse* response) override;

  [[nodiscard]] grpc::Status UpdateSelectedFunctions(
      grpc::ServerContext* context,
      const orbit_grpc_protos::UpdateSelectedFunctionsRequest* request,
      orbit_grpc_protos::UpdateSelectedFunctionsResponse* response) override;

  [[nodiscard]] grpc::Status ShutdownService(
      grpc::ServerContext* context, const orbit_grpc_protos::ShutdownServiceRequest* request,
      orbit_grpc_protos::ShutdownServiceResponse* response) override;

  [[nodiscard]] bool ShutdownFinished() const;

 private:
  std::unique_ptr<ClientGgp> client_ggp_;
  std::shared_ptr<orbit_base::ThreadPool> thread_pool_;
  std::atomic<bool> shutdown_finished_{false};

  void InitClientGgp();
  void Shutdown();
  bool CaptureIsRunning();
};

#endif  // ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_IMPL_H_
