// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/SessionManager.h"

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Session.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

SessionManager::SessionManager(Credentials credentials)
    : credentials_(credentials) {}

// Tick establishes a ssh connection via different states and progresses this
// state when appropriate. Once kAuthenticated is reached, a connection is
// established.
// The states are the following:
// * kNotInitialized: A socket is not created yet
// * kSocketCreated: A socket exists
// * kSocketConnected: The socket is connected to the remote host
// * kSessionCreated: The ssh session is initialized.
// * kHandshaked: The ssh handshake has happened.
// * kMatchedKnownHosts: the remote server has been successfully matched with
// the known hosts file
// * kAuthenticated: The authentication was successful.
SessionManager::State SessionManager::Tick() {
  switch (state_) {
    case State::kNotInitialized: {
      std::optional<Socket> new_socket_opt = Socket::Create();
      if (new_socket_opt.has_value()) {
        socket_ = std::move(new_socket_opt);
        state_ = State::kSocketCreated;
      }
      break;
    }
    case State::kSocketCreated: {
      if (socket_->Connect(credentials_.host, credentials_.port) ==
          ResultType::kSuccess) {
        state_ = State::kSocketConnected;
      }
      break;
    }
    case State::kSocketConnected: {
      std::optional<Session> session_opt = Session::Create();
      if (session_opt.has_value()) {
        session_ = std::move(session_opt);
        state_ = State::kSessionCreated;
        session_->SetBlocking(false);
      }
      break;
    }
    case State::kSessionCreated: {
      if (session_->Handshake(&socket_.value()) == ResultType::kSuccess) {
        state_ = State::kHandshaked;
      }
      break;
    }
    case State::kHandshaked: {
      if (session_->MatchKnownHosts(credentials_.host, credentials_.port,
                                    credentials_.known_hosts_path) ==
          ResultType::kSuccess) {
        state_ = State::kMatchedKnownHosts;
      }
      break;
    }
    case State::kMatchedKnownHosts: {
      if (session_->Authenticate(credentials_.user, credentials_.key_path) ==
          ResultType::kSuccess) {
        state_ = State::kAuthenticated;
      }
      break;
    }
    case State::kAuthenticated: {
      break;
    }
  }

  return state_;
}

// The Close function gracefully closes the session and the socket used by the
// session. This is dependent on the state the session is in, aka how far the
// establishing of a connection already progressed.
ResultType SessionManager::Close() {
  ResultType result;
  switch (state_) {
    // disconnect session
    case State::kAuthenticated:
    case State::kMatchedKnownHosts:
    case State::kHandshaked:
    case State::kSessionCreated:
      result = session_->Disconnect();
      if (result != ResultType::kSuccess) {
        return result;
        session_ = std::nullopt;
      }

      state_ = State::kSocketConnected;

    // shutdown socket
    case State::kSocketConnected:
      result = socket_->Shutdown();
      if (result != ResultType::kSuccess) return result;
      state_ = State::kSocketCreated;

    // close socket
    case State::kSocketCreated:
      socket_ = std::nullopt;
      state_ = State::kNotInitialized;

    // do nothing
    case State::kNotInitialized:
      break;
  }

  state_ = State::kNotInitialized;
  return ResultType::kSuccess;
}

}  // namespace OrbitSsh