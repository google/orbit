// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SESSIONMANAGER_H_
#define ORBIT_SSH_SESSIONMANAGER_H_

#include <filesystem>

#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Session.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

// SessionManager manages a ssh session and its lifetime. To establish a ssh
// connection, the  Tick() function has to be called periodically. This creates
// and connect the necessary socket, initializes the ssh session and performs
// handshake, known_hosts_matching and authentication.
// This class is constructed with the ssh credentials needed to establish a
// connection.
class SessionManager {
 public:
  enum class State {
    kNotInitialized,
    kSocketCreated,
    kSocketConnected,
    kSessionCreated,
    kHandshaked,
    kMatchedKnownHosts,
    kAuthenticated
  };

  explicit SessionManager(Credentials credentials);
  SessionManager() = delete;
  SessionManager(const SessionManager&) = delete;
  SessionManager& operator=(const SessionManager&) = delete;

  State Tick();
  Session* GetSessionPtr() {
    return session_.has_value() ? &session_.value() : nullptr;
  }
  Socket* GetSocketPtr() { return &socket_.value(); }
  ResultType Close();

 private:
  State state_ = State::kNotInitialized;
  std::optional<Socket> socket_;
  std::optional<Session> session_;
  Credentials credentials_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SESSIONMANAGER_H_
