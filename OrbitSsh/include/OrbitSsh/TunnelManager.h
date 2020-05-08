// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_PORT_FORWARD_MANAGER_H_
#define ORBIT_SSH_PORT_FORWARD_MANAGER_H_

#include <memory>

#include "OrbitSsh/DirectTcpIpChannelManager.h"
#include "OrbitSsh/LocalSocketManager.h"
#include "OrbitSsh/ResultType.h"

namespace OrbitSsh {

// This class manages a ssh tunnel from a local tcp/ip socket to a tcp/ip socket
// on the remote ssh server. The tunnel is set up with a local port on which it
// listens and a remote port that it connects to on the ssh server. This is done
// by using LocalSocketManager to manage the local socket and DirectTcpIpManager
// to manage the connection to the remote. Tick needs to be called periodically
// to move data from socket to channel and vice versa.
class TunnelManager {
 public:
  enum class State { kNotConnected, kConnected };

  explicit TunnelManager(Session* session_ptr, int local_port, int remote_port);
  TunnelManager(const TunnelManager&) = delete;
  TunnelManager& operator=(const TunnelManager&) = delete;
  TunnelManager(TunnelManager&&) = default;
  TunnelManager& operator=(TunnelManager&&) = default;

  State Tick();
  ResultType Close();

 private:
  ResultType Connect();
  ResultType ReceiveSocketWriteChannel();
  ResultType ReadChannelSendSocket();
  State state_ = State::kNotConnected;
  LocalSocketManager socket_;
  DirectTcpIpChannelManager channel_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_PORT_FORWARD_MANAGER_H_