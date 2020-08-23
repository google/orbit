// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTMANAGER_H
#define ORBIT_TRACEPOINTMANAGER_H

#include <absl/time/time.h>
#include <grpcpp/channel.h>

#include <vector>

#include "../OrbitService/TracepointServiceImpl.h"
#include "OrbitBase/Result.h"
#include "tracepoint.pb.h"

class TracepointManager {
 public:
  TracepointManager() = default;
  virtual ~TracepointManager() = default;

  static std::unique_ptr<TracepointManager> Create(const std::shared_ptr<grpc::Channel>& channel,
                                                absl::Duration refresh_timeout);

  virtual ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> LoadTracepointList() = 0;

  virtual std::vector<orbit_grpc_protos::TracepointInfo> GetTracepointList() const = 0;

};

#endif  // ORBIT_TRACEPOINTMANAGER_H
