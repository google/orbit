// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTSERVICECLIENT_H
#define ORBIT_TRACEPOINTSERVICECLIENT_H

#include <vector>

#include "OrbitBase/Result.h"
#include "grpcpp/channel.h"
#include "services.grpc.pb.h"
#include "tracepoint.pb.h"

using orbit_grpc_protos::TracepointInfo;

class TracepointServiceClient {
 public:
  virtual ~TracepointServiceClient() = default;

  static std::unique_ptr<TracepointServiceClient> Create(
      const std::shared_ptr<grpc::Channel>& channel);

  virtual std::vector<TracepointInfo> GetTracepointList() const;

  void PopulateWithServerData();
  void Start();

  explicit TracepointServiceClient(const std::shared_ptr<grpc::Channel>& channel);

 private:
  TracepointServiceClient() = default;

  std::unique_ptr<grpc::ClientContext> CreateContext() const;

  std::unique_ptr<orbit_grpc_protos::TracepointService::Stub> tracepoint_service_;

  std::vector<TracepointInfo> tracepoint_list_;
};

#endif  // ORBIT_TRACEPOINTSERVICECLIENT_H
