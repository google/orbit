// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/TunnelManager.h"

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/DirectTcpIpChannelManager.h"
#include "OrbitSsh/LocalSocketManager.h"

namespace OrbitSsh {

TunnelManager::TunnelManager(Session* session_ptr, int local_port,
                             int remote_port)
    : session_ptr_(session_ptr),
      local_port_(local_port),
      remote_port_(remote_port) {}

outcome::result<void> TunnelManager::Connect() {
  const std::string local_ip_addr = "127.0.0.1";
  if (!socket_) {
    socket_.emplace(local_ip_addr, local_port_);
  }

  if (!channel_) {
    channel_.emplace(session_ptr_, local_ip_addr, remote_port_);
  }

  OUTCOME_TRY(socket_->Connect());
  OUTCOME_TRY(channel_->Initialize());
  return outcome::success();
}

outcome::result<void> TunnelManager::ReceiveSocketWriteChannel() {
  OUTCOME_TRY(data, socket_->Receive());
  if (data.empty()) {
    return std::error_code{ENOTCONN, std::system_category()};
  }

  OUTCOME_TRY(channel_->WriteBlocking(data));
  return outcome::success();
}

outcome::result<void> TunnelManager::ReadChannelSendSocket() {
  OUTCOME_TRY(data, channel_->Read());
  if (data.empty()) {
    return std::error_code{ENOTCONN, std::system_category()};
  }

  OUTCOME_TRY(socket_->SendBlocking(data));
  return outcome::success();
}

// This function keeps trying to make a connection when its in the kNotConnected
// state. Once the connection is established, it exchanges packages between the
// local socket (managed by LocalSocketManager) and the channel to the remote
// socket (managed by DirectTcpIpManager). This is done by first receiving from
// the socket and writing to the channel and then reading from the channel and
// sending to the socket. If either of these operations fails, the state will be
// set back to kNotConnected.
outcome::result<void> TunnelManager::Tick() {
  switch (state_) {
    case State::kNotConnected: {
      OUTCOME_TRY(Connect());
      state_ = State::kConnected;
    }
    case State::kConnected: {
      if (const auto result = ReceiveSocketWriteChannel(); result.has_error()) {
        state_ = State::kNotConnected;
        socket_ = std::nullopt;
        channel_ = std::nullopt;
        return result;
      }
      if (const auto result = ReadChannelSendSocket(); result.has_error()) {
        // The Ui needs to realize the connection was interrupted, so it starts
        // send hello messages again;
        state_ = State::kNotConnected;
        OUTCOME_TRY(socket_->ForceReconnect());
        return result;
      }
      break;
    }
  }

  return outcome::success();
}

outcome::result<void> TunnelManager::Close() {
  OUTCOME_TRY(channel_->Close());
  OUTCOME_TRY(socket_->Close());
  return outcome::success();
}

}  // namespace OrbitSsh
