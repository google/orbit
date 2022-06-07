// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_SESSION_SETUP_UTILS_H_
#define SESSION_SETUP_SESSION_SETUP_UTILS_H_

#include <grpcpp/create_channel.h>

#include <QString>
#include <memory>
#include <optional>

#include "ClientData/ProcessData.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitGgp/SshInfo.h"
#include "OrbitSsh/Credentials.h"

namespace orbit_session_setup {

const int kMaxProcessNameLength = 15;
const QString kCustomProtocol = "orbitprofiler";
const QString kCustomProtocolDelimiter = "://";

struct ConnectionTarget {
  QString process_name_or_path;
  QString instance_name_or_id;

  ConnectionTarget(const QString& process_name_or_path, const QString& instance_name_or_id)
      : process_name_or_path(process_name_or_path), instance_name_or_id(instance_name_or_id) {}
};

[[nodiscard]] orbit_ssh::Credentials CredentialsFromSshInfo(const orbit_ggp::SshInfo& ssh_info);
[[nodiscard]] std::shared_ptr<grpc::Channel> CreateGrpcChannel(uint16_t port);
[[nodiscard]] std::unique_ptr<orbit_client_data::ProcessData> TryToFindProcessData(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list,
    const std::string& process_name_or_path);

// Split a string of format "orbitprofiler://instance?process
[[nodiscard]] std::optional<ConnectionTarget> SplitTargetUri(const QString& target_uri);

}  // namespace orbit_session_setup

#endif