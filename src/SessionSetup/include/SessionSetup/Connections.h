// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_CONNECTIONS_H_
#define SESSION_SETUP_CONNECTIONS_H_

#include <absl/time/time.h>
#include <grpcpp/channel.h>

#include <memory>
#include <optional>
#include <utility>

#include "ClientServices/ProcessManager.h"
#include "DeploymentConfigurations.h"
#include "OrbitBase/Logging.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Context.h"
#include "SessionSetup/OrbitServiceInstance.h"
#include "SessionSetup/ServiceDeployManager.h"

namespace orbit_session_setup {

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
    ORBIT_CHECK(ssh_context != nullptr);
    ORBIT_CHECK(deployment_configuration != nullptr);
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
 * The class `SshConnection` describes an active connection to a machine via ssh. This class holds
 * a `AddrAndPort` which is the target of the ssh connection. The `ServiceDeployManager` which
 * carries the active connection and the ssh tunnel. And the grpc channel that is used for the
 * communication with the machine. This class is meant to be constructed and then not modified
 * anymore. Only `SshConnectionWidget` is allowed to modify the members, which is used to move out
 * members for reusing them.
 */
class SshConnection {
 public:
  explicit SshConnection(orbit_ssh::AddrAndPort addr_and_port,
                         std::unique_ptr<ServiceDeployManager> service_deploy_manager,
                         std::shared_ptr<grpc::Channel>&& grpc_channel)
      : addr_and_port_(std::move(addr_and_port)),
        service_deploy_manager_(std::move(service_deploy_manager)),
        grpc_channel_(std::move(grpc_channel)),
        process_manager_(orbit_client_services::ProcessManager::Create(grpc_channel_,
                                                                       absl::Milliseconds(1000))) {
    ORBIT_CHECK(service_deploy_manager_ != nullptr);
    ORBIT_CHECK(grpc_channel_ != nullptr);
    ORBIT_CHECK(process_manager_ != nullptr);
  }
  [[nodiscard]] const orbit_ssh::AddrAndPort& GetAddrAndPort() const { return addr_and_port_; }
  [[nodiscard]] ServiceDeployManager* GetServiceDeployManager() const {
    return service_deploy_manager_.get();
  }
  [[nodiscard]] const std::shared_ptr<grpc::Channel>& GetGrpcChannel() const {
    return grpc_channel_;
  }
  [[nodiscard]] orbit_client_services::ProcessManager* GetProcessManager() const {
    return process_manager_.get();
  }

 private:
  orbit_ssh::AddrAndPort addr_and_port_;
  std::unique_ptr<ServiceDeployManager> service_deploy_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
  std::unique_ptr<orbit_client_services::ProcessManager> process_manager_;
};

/*
 * The LocalConnection class describes an active connection to an OrbitService running on the same
 * machine as the UI. This class holds a grpc channel which is used for the communication with
 * OrbitService and an optional OrbitServiceInstance. Optional here means that the
 * unique_ptr<OrbitServiceInstance> can be a nullptr. This class is meant to be constructed and then
 * not modified anymore. Only ConnectToLocalWidget is allowed to modify the members, which is used
 * to move out members for reusing them.
 */
class LocalConnection {
  friend class ConnectToLocalWidget;

 public:
  explicit LocalConnection(std::shared_ptr<grpc::Channel>&& grpc_channel,
                           std::unique_ptr<OrbitServiceInstance>&& orbit_service_instance)
      : grpc_channel_(std::move(grpc_channel)),
        orbit_service_instance_(std::move(orbit_service_instance)),
        process_manager_(orbit_client_services::ProcessManager::Create(grpc_channel_,
                                                                       absl::Milliseconds(1000))) {
    ORBIT_CHECK(grpc_channel_ != nullptr);
    ORBIT_CHECK(process_manager_ != nullptr);
  }
  [[nodiscard]] const std::shared_ptr<grpc::Channel>& GetGrpcChannel() const {
    return grpc_channel_;
  }
  [[nodiscard]] const OrbitServiceInstance* GetOrbitServiceInstance() const {
    return orbit_service_instance_.get();
  }
  [[nodiscard]] orbit_client_services::ProcessManager* GetProcessManager() const {
    return process_manager_.get();
  }

 private:
  std::shared_ptr<grpc::Channel> grpc_channel_;
  std::unique_ptr<OrbitServiceInstance> orbit_service_instance_;
  std::unique_ptr<orbit_client_services::ProcessManager> process_manager_;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_CONNECTIONS_H_
