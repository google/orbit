// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_SESSION_SETUP_UTILS_H_
#define SESSION_SETUP_SESSION_SETUP_UTILS_H_

#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <stdint.h>

#include <QString>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/ProcessData.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitSsh/Credentials.h"

namespace orbit_session_setup {

const int kMaxProcessNameLength = 15;

struct ConnectionTarget {
  QString process_name_or_path;
  orbit_ssh::Credentials credentials;

  ConnectionTarget(QString process_name_or_path, orbit_ssh::Credentials credentials)
      : process_name_or_path(std::move(process_name_or_path)),
        credentials(std::move(credentials)) {}
};

[[nodiscard]] std::shared_ptr<grpc::Channel> CreateGrpcChannel(uint16_t port);
[[nodiscard]] std::unique_ptr<orbit_client_data::ProcessData> TryToFindProcessData(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list,
    std::string_view process_name_or_path);

// Split a string of format "orbitprofiler://instance?process
[[nodiscard]] std::optional<ConnectionTarget> SplitTargetUri(const QString& target_uri);

}  // namespace orbit_session_setup

#endif