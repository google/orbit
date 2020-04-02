// TODO(http://b/148520406): Add copyright here
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORIBIT_SERIVICE_PROCESS_SERVICE_IMPL_H_
#define ORIBIT_SERIVICE_PROCESS_SERVICE_IMPL_H_

#include <memory>
#include <string>

#include "ProcessUtils.h"
#include "services.grpc.pb.h"

class ProcessServiceImpl final : public ProcessService::Service {
 public:
  grpc::Status GetProcessList(grpc::ServerContext* context,
                              const GetProcessListRequest* request,
                              GetProcessListReply* reply) override;
 private:
  ProcessList process_list_;
};

#endif  // ORIBIT_SERIVICE_PROCESS_SERVICE_IMPL_H_
