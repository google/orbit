// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERIVICE_PROCESS_SERVICE_IMPL_H_
#define ORBIT_SERIVICE_PROCESS_SERVICE_IMPL_H_

#include <memory>
#include <string>

#include "ProcessUtils.h"
#include "services.grpc.pb.h"

class ProcessServiceImpl final : public ProcessService::Service {
 public:
  grpc::Status GetProcessList(grpc::ServerContext* context,
                              const GetProcessListRequest* request,
                              GetProcessListResponse* response) override;
 private:
  ProcessList process_list_;
};

#endif  // ORBIT_SERIVICE_PROCESS_SERVICE_IMPL_H_
