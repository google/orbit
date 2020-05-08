// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/TunnelManager.h"

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/DirectTcpIpChannelManager.h"
#include "OrbitSsh/LocalSocketManager.h"
#include "OrbitSsh/ResultType.h"

namespace OrbitSsh {

TunnelManager::TunnelManager(Session* session_ptr, int local_port,
                             int remote_port)
    : socket_("127.0.0.1", local_port),
      channel_(session_ptr, "127.0.0.1", remote_port) {}

ResultType TunnelManager::Connect() {
  LocalSocketManager::State socket_state = socket_.Tick();
  DirectTcpIpChannelManager::State channel_state = channel_.Tick();

  if (socket_state == LocalSocketManager::State::kRunning &&
      channel_state == DirectTcpIpChannelManager::State::kRunning) {
    return ResultType::kSuccess;
  }
  return ResultType::kAgain;
}

ResultType TunnelManager::ReceiveSocketWriteChannel() {
  std::string text;
  ResultType result = socket_.Receive(&text);

  if (result != ResultType::kSuccess) return result;

  return channel_.WriteBlocking(text);
}

ResultType TunnelManager::ReadChannelSendSocket() {
  std::string text;
  ResultType result = channel_.Read(&text);

  if (result != ResultType::kSuccess) return result;

  return socket_.SendBlocking(text);
}

// This function keeps trying to make a connection when its in the kNotConnected
// state. Once the connection is established, it exchanges packages between the
// local socket (managed by LocalSocketManager) and the channel to the remote
// socket (managed by DirectTcpIpManager). This is done by first receiving from
// the socket and writing to the channel and then reading from the channel and
// sending to the socket. If either of these operations fails, the state will be
// set back to kNotConnected.
TunnelManager::State TunnelManager::Tick() {
  ResultType result = ResultType::kAgain;
  switch (state_) {
    case State::kNotConnected: {
      result = Connect();
      if (result == ResultType::kSuccess) state_ = State::kConnected;
      break;
    }
    case State::kConnected: {
      ResultType result = ReceiveSocketWriteChannel();
      if (result == ResultType::kError) {
        state_ = State::kNotConnected;
        break;
      }
      result = ReadChannelSendSocket();
      if (result == ResultType::kError) {
        // The Ui needs to realize the connection was interrupted, so it starts
        // send hello messages again;
        socket_.ForceReconnect();
        state_ = State::kNotConnected;
      }
      break;
    }
  }

  return state_;
}

ResultType TunnelManager::Close() {
  ResultType result_channel = channel_.Close();
  ResultType result_socket = socket_.Close();

  if (result_channel == ResultType::kError ||
      result_socket == ResultType::kError) {
    return ResultType::kError;
  }

  if (result_channel == ResultType::kSuccess &&
      result_socket == ResultType::kSuccess) {
    return ResultType::kSuccess;
  }

  return ResultType::kAgain;
}

}  // namespace OrbitSsh