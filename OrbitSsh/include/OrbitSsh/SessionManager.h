// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SESSIONMANAGER_H_
#define ORBIT_SSH_SESSIONMANAGER_H_

#include <filesystem>
#include <outcome.hpp>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
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
  enum class State {
    kNotInitialized,
    kSocketCreated,
    kSocketConnected,
    kSessionCreated,
    kHandshaked,
    kMatchedKnownHosts,
    kAuthenticated
  };

 public:
  explicit SessionManager(Context* context, Credentials credentials);

  SessionManager(const SessionManager&) = delete;
  SessionManager& operator=(const SessionManager&) = delete;

  outcome::result<void> Initialize();

  bool isInitialized() const noexcept {
    return state_ == State::kAuthenticated;
  }

  Session* GetSessionPtr() {
    CHECK(session_);
    return &session_.value();
  }
  Socket* GetSocketPtr() {
    CHECK(socket_);
    return &socket_.value();
  }

  outcome::result<void> Close();

 private:
  State state_ = State::kNotInitialized;
  std::optional<Socket> socket_;
  std::optional<Session> session_;
  Credentials credentials_;
  Context* context_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SESSIONMANAGER_H_
