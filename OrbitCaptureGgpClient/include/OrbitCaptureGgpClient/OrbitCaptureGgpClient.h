// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_GGP_CLIENT_ORBIT_CAPTURE_GGP_CLIENT_H_
#define ORBIT_CAPTURE_GGP_CLIENT_ORBIT_CAPTURE_GGP_CLIENT_H_

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <memory>
#include <string>

#include "services_ggp.grpc.pb.h"

class CaptureClientGgpClient {
 public:
  CaptureClientGgpClient(std::shared_ptr<grpc::Channel> channel)
      : capture_client_ggp_service_(orbit_grpc_protos::CaptureClientGgpService::NewStub(channel)){};

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user);

 private:
  std::unique_ptr<orbit_grpc_protos::CaptureClientGgpService::Stub> capture_client_ggp_service_;
};

#endif  // ORBIT_CAPTURE_GGP_CLIENT_ORBIT_CAPTURE_GGP_CLIENT_H_