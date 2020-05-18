// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/SessionManager.h"

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/Session.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

SessionManager::SessionManager(Context* context, Credentials credentials)
    : credentials_(credentials), context_(context) {}

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
outcome::result<void> SessionManager::Initialize() {
  switch (state_) {
    case State::kNotInitialized: {
      OUTCOME_TRY(socket, Socket::Create());
      socket_ = std::move(socket);
      state_ = State::kSocketCreated;
    }
    case State::kSocketCreated: {
      OUTCOME_TRY(socket_->Connect(credentials_.host, credentials_.port));
      state_ = State::kSocketConnected;
    }
    case State::kSocketConnected: {
      OUTCOME_TRY(session, Session::Create(context_));
      session_ = std::move(session);
      session_->SetBlocking(false);
      state_ = State::kSessionCreated;
    }
    case State::kSessionCreated: {
      OUTCOME_TRY(session_->Handshake(&socket_.value()));
      state_ = State::kHandshaked;
    }
    case State::kHandshaked: {
      OUTCOME_TRY(session_->MatchKnownHosts(
          credentials_.host, credentials_.port, credentials_.known_hosts_path));
      state_ = State::kMatchedKnownHosts;
    }
    case State::kMatchedKnownHosts: {
      OUTCOME_TRY(
          session_->Authenticate(credentials_.user, credentials_.key_path));
      state_ = State::kAuthenticated;
    }
    case State::kAuthenticated: {
      break;
    }
  }

  return outcome::success();
}

// The Close function gracefully closes the session and the socket used by the
// session. This is dependent on the state the session is in, aka how far the
// establishing of a connection already progressed.
outcome::result<void> SessionManager::Close() {
  switch (state_) {
    // disconnect session
    case State::kAuthenticated:
    case State::kMatchedKnownHosts:
    case State::kHandshaked:
    case State::kSessionCreated: {
      OUTCOME_TRY(session_->Disconnect());
      state_ = State::kSocketConnected;
    }

    // shutdown socket
    case State::kSocketConnected: {
      OUTCOME_TRY(socket_->Shutdown());
      state_ = State::kSocketCreated;
    }

    // close socket
    case State::kSocketCreated:
      socket_ = std::nullopt;
      state_ = State::kNotInitialized;

    // do nothing
    case State::kNotInitialized:
      break;
  }

  return outcome::success();
}

}  // namespace OrbitSsh
