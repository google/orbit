// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/SessionSetupUtils.h"

#include <QStringList>

#include "OrbitBase/Logging.h"

namespace orbit_session_setup {

orbit_ssh::Credentials CredentialsFromSshInfo(const orbit_ggp::SshInfo& ssh_info) {
  orbit_ssh::Credentials credentials;
  credentials.addr_and_port = {ssh_info.host.toStdString(), ssh_info.port};
  credentials.key_path = ssh_info.key_path.toStdString();
  credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
  credentials.user = ssh_info.user.toStdString();

  return credentials;
}

std::shared_ptr<grpc::Channel> CreateGrpcChannel(uint16_t port) {
  std::string grpc_server_address = absl::StrFormat("127.0.0.1:%d", port);
  LOG("Starting gRPC channel to: %s", grpc_server_address);
  std::shared_ptr<grpc::Channel> result = grpc::CreateCustomChannel(
      grpc_server_address, grpc::InsecureChannelCredentials(), grpc::ChannelArguments());
  CHECK(result != nullptr);
  return result;
}

std::optional<ConnectionTarget> orbit_session_setup::ConnectionTarget::FromString(
    const QString& connection_target) {
  auto parts = connection_target.split('@', QString::SplitBehavior::KeepEmptyParts);

  if (parts.size() != 2) {
    return std::nullopt;
  }

  bool ok = false;
  uint pid = parts[0].toUInt(&ok);
  if (!ok) {
    return std::nullopt;
  }

  return ConnectionTarget{parts[1], pid};
}

}  // namespace orbit_session_setup