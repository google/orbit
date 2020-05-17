// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_FORWARD_SOCKET_MANAGER_H_
#define ORBIT_SSH_FORWARD_SOCKET_MANAGER_H_

#include <memory>
#include <outcome.hpp>
#include <string>

#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

// LocalSocketManager manages a localhost tcp/ip socket. It bindes the socket to
// a specified address and port and starts listening. Once another party
// connects to the socket, it accepts the connection. When the other party
// disconnects it goes back to listening. The Tick function needs to be called
// periodically to progress through the different states.
class LocalSocketManager {
 public:
  explicit LocalSocketManager(std::string address, int port);

  // Tick manages the current state and progresses when appropriate.
  // * kNotInitialized means both listen_socket and accepted_socket are not
  // initialized aka nullptr. In this state it is tried to create a
  // listen_socket.
  // * kInitialized means the listen_socket exists and it is tried to bind it to
  // the address and port
  // * kBound means listen_socket bind was successful and the next step is start
  // listening
  // * kListening means the listen_socket is listening until a connection on the
  // socket can be accepted
  // * kRunning means there is an active connection on the accepted_socket.
  // * kShutdownSent means a shutdown signal has been sent and the
  // accepted_channel waits on a disconnect signal to arrive.
  // * kRemoteDisconnect means there is no active connection on the accepted
  // socket anymore. The socket will be deleted and the state goes back to
  // listening for new connections (kListening)
  outcome::result<void> Connect();
  outcome::result<std::string> Receive();
  outcome::result<void> SendBlocking(std::string_view text);
  outcome::result<void> ForceReconnect();
  outcome::result<void> Close();

 private:
  enum class State {
    kNotInitialized,
    kInitialized,
    kBound,
    kListening,
    kRunning,
    kShutdownSent,
    kRemoteDisconnected
  };

  outcome::result<void> Accept();
  State state_ = State::kNotInitialized;
  std::optional<Socket> accepted_socket_;
  std::optional<Socket> listen_socket_;
  std::string address_;
  int port_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_FORWARD_SOCKET_MANAGER_H_
