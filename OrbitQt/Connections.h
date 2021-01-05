// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CONNECTIONS_H_
#define ORBIT_QT_CONNECTIONS_H_

#include <memory>
#include <optional>
#include <utility>

#include "DeploymentConfigurations.h"
#include "OrbitBase/Logging.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OrbitGgp/Instance.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "grpcpp/channel.h"
#include "servicedeploymanager.h"

namespace orbit_qt {

/*
 * This class holds data that is required to establish a ssh connection. This includes the context,
 * which handles the encryption underlying the encryption, the grpc port on the remote server that
 * should be used and a deployment configuration, which holds information about how OrbitService
 * should be deployed.
 * This class is meant to be constructed once and then not get modified during the usage of Orbit.
 */
class SshConnectionArtifacts {
 public:
  explicit SshConnectionArtifacts(const orbit_ssh::Context* ssh_context,
                                  ServiceDeployManager::GrpcPort grpc_port,
                                  const DeploymentConfiguration* deployment_configuration)
      : ssh_context_(ssh_context),
        grpc_port_(grpc_port),
        deployment_configuration_(deployment_configuration) {
    CHECK(ssh_context != nullptr);
    CHECK(deployment_configuration != nullptr);
  }

  [[nodiscard]] const orbit_ssh::Context* GetSshContext() const { return ssh_context_; }
  [[nodiscard]] const ServiceDeployManager::GrpcPort& GetGrpcPort() const { return grpc_port_; }
  [[nodiscard]] const DeploymentConfiguration* GetDeploymentConfiguration() const {
    return deployment_configuration_;
  }

 private:
  const orbit_ssh::Context* ssh_context_;
  const ServiceDeployManager::GrpcPort grpc_port_;
  const DeploymentConfiguration* deployment_configuration_;
};

/*
 * The class StadiaConnection describes an active connection to a stadia instance. This class holds
 * an Instance object it is connected. The ServiceDeployManager which carries the active connection
 * and the ssh tunnel. And the grpc channel that is used for the communication with the instance.
 * This class is meant to be constructed and then not modified anymore. Only ConnectToStadiaWidget
 * is allowed to modify the members, which is used to move out members for reusing them.
 */
class StadiaConnection {
  friend class ConnectToStadiaWidget;

 public:
  explicit StadiaConnection(orbit_ggp::Instance&& instance,
                            std::unique_ptr<ServiceDeployManager> service_deploy_manager,
                            std::shared_ptr<grpc::Channel>&& grpc_channel)
      : instance_(std::move(instance)),
        service_deploy_manager_(std::move(service_deploy_manager)),
        grpc_channel_(std::move(grpc_channel)) {
    CHECK(service_deploy_manager_ != nullptr);
    CHECK(grpc_channel_ != nullptr);
  }
  [[nodiscard]] const orbit_ggp::Instance& GetInstance() const { return instance_; }
  [[nodiscard]] ServiceDeployManager* GetServiceDeployManager() const {
    return service_deploy_manager_.get();
  }
  [[nodiscard]] const std::shared_ptr<grpc::Channel>& GetGrpcChannel() const {
    return grpc_channel_;
  }

 private:
  orbit_ggp::Instance instance_;
  std::unique_ptr<ServiceDeployManager> service_deploy_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
};

/*
 * The LocalConnection class describes an active connection to an OrbitService running on the same
 * machine as the UI. This class holds a grpc channel which is used for the communication with
 * OrbitService.
 * This class is meant to be constructed and then not modified anymore.
 */
class LocalConnection {
 public:
  explicit LocalConnection(std::shared_ptr<grpc::Channel>&& grpc_channel)
      : grpc_channel_(std::move(grpc_channel)) {
    CHECK(grpc_channel_ != nullptr);
  }
  [[nodiscard]] const std::shared_ptr<grpc::Channel>& GetGrpcChannel() const {
    return grpc_channel_;
  }

 private:
  std::shared_ptr<grpc::Channel> grpc_channel_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_CONNECTIONS_H_
