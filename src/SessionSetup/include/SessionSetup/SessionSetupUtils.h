// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_SESSION_SETUP_UTILS_H_
#define SESSION_SETUP_SESSION_SETUP_UTILS_H_

#include <grpcpp/create_channel.h>

#include <memory>
#include <optional>

#include "OrbitGgp/SshInfo.h"
#include "OrbitSsh/Credentials.h"

namespace orbit_session_setup {

[[nodiscard]] orbit_ssh::Credentials CredentialsFromSshInfo(const orbit_ggp::SshInfo& ssh_info);
[[nodiscard]] std::shared_ptr<grpc::Channel> CreateGrpcChannel(uint16_t port);

// Split a target given as "<pid>@<instance_id>" into its components
[[nodiscard]] std::optional<std::pair<QString, uint32_t>>
SplitInstanceAndProcessIdFromConnectionTarget(const QString& connection_target);

}  // namespace orbit_session_setup

#endif