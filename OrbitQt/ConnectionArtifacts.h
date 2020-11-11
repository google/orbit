// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CONNECTION_ARTIFACTS_H_
#define ORBIT_QT_CONNECTION_ARTIFACTS_H_

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OrbitGgp/Instance.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "deploymentconfigurations.h"
#include "grpcpp/channel.h"
#include "servicedeploymanager.h"

namespace OrbitQt {

// A class to store remote connection related artifacts. This is used by the ProfilingTargetDialog
// and OrbitApp to be able to use the same connection. The
class ConnectionArtifacts {
 public:
  ConnectionArtifacts(const OrbitSsh::Context* ssh_context,
                      const ServiceDeployManager::GrpcPort& grpc_port,
                      const DeploymentConfiguration* deployment_configuration)
      : ssh_context_(ssh_context),
        grpc_port_(grpc_port),
        deployment_configuration_(deployment_configuration) {
    CHECK(ssh_context_ != nullptr);
    CHECK(deployment_configuration_ != nullptr);
  }
  void CreateServiceDeployManager(OrbitSsh::Credentials credentials) {
    CHECK(service_deploy_manager_ == nullptr);
    service_deploy_manager_ = std::make_unique<ServiceDeployManager>(
        deployment_configuration_, ssh_context_, std::move(credentials), grpc_port_);
  }

  // Required Member
  const OrbitSsh::Context* ssh_context_;
  const ServiceDeployManager::GrpcPort& grpc_port_;
  const DeploymentConfiguration* deployment_configuration_;

  // Optional Member
  std::unique_ptr<ServiceDeployManager> service_deploy_manager_;
  std::optional<OrbitGgp::Instance> selected_instance_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
  std::unique_ptr<ProcessManager> process_manager_;
  std::unique_ptr<ProcessData> process_;
};

}  // namespace OrbitQt

#endif  // ORBIT_QT_CONNECTION_ARTIFACTS_H_
