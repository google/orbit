// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpServiceImpl.h"

grpc::Status CaptureClientGgpServiceImpl::SayHello(grpc::ServerContext*,
                                                   const orbit_grpc_protos::HelloRequest* request,
                                                   orbit_grpc_protos::HelloReply* reply) {
  std::string prefix("Hello ");
  reply->set_message(prefix + request->name());
  return grpc::Status::OK;
}