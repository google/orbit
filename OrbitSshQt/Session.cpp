// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/Session.h"

#include <optional>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Session.h"
#include "OrbitSshQt/Error.h"

namespace OrbitSshQt {

outcome::result<void> Session::startup() {
  switch (CurrentState()) {
    case State::kInitial:
    case State::kDisconnected: {
      OUTCOME_TRY(socket, OrbitSsh::Socket::Create());
      socket_ = std::move(socket);
      notifiers_.emplace(socket_->GetFileDescriptor(), this);
      QObject::connect(&notifiers_->read, &QSocketNotifier::activated, this, &Session::OnEvent);
      QObject::connect(&notifiers_->write, &QSocketNotifier::activated, this, &Session::OnEvent);
      SetState(State::kSocketCreated);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kSocketCreated: {
      OUTCOME_TRY(socket_->Connect(credentials_.addr_and_port));
      SetState(State::kSocketConnected);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kSocketConnected: {
      OUTCOME_TRY(session, OrbitSsh::Session::Create(context_));
      session_ = std::move(session);
      session_->SetBlocking(false);
      SetState(State::kSessionCreated);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kSessionCreated: {
      OUTCOME_TRY(session_->Handshake(&socket_.value()));
      SetState(State::kHandshaked);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kHandshaked: {
      OUTCOME_TRY(
          session_->MatchKnownHosts(credentials_.addr_and_port, credentials_.known_hosts_path));
      SetState(State::kMatchedKnownHosts);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kMatchedKnownHosts: {
      OUTCOME_TRY(session_->Authenticate(credentials_.user, credentials_.key_path));
      SetState(State::kConnected);
      break;
    }
    case State::kStarted:
    case State::kConnected:
    case State::kShutdown:
    case State::kAboutToDisconnect:
    case State::kDone:
    case State::kError:
      UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> Session::shutdown() {
  switch (CurrentState()) {
    case State::kInitial:
    case State::kDisconnected:
    case State::kSocketCreated:
    case State::kSocketConnected:
    case State::kSessionCreated:
    case State::kHandshaked:
    case State::kMatchedKnownHosts:
    case State::kStarted:
    case State::kConnected:
      UNREACHABLE();
    case State::kShutdown:
    case State::kAboutToDisconnect: {
      OUTCOME_TRY(session_->Disconnect());
      notifiers_ = std::nullopt;
      socket_ = std::nullopt;
      session_ = std::nullopt;
      SetState(State::kDone);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kDone:
      break;
    case State::kError:
      UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> Session::run() {
  emit dataEvent();
  return outcome::success();
}

void Session::HandleEagain() {
  if (notifiers_) {
    const int flags = libssh2_session_block_directions(session_->GetRawSessionPtr());

    // When any of libssh2 functions return LIBSSH2_ERROR_EAGAIN an application
    // should wait for the socket to have data available for reading or writing.
    // `libssh2_session_block_directions` tells which direction to listen on.

    notifiers_->read.setEnabled(flags & LIBSSH2_SESSION_BLOCK_INBOUND);
    notifiers_->write.setEnabled(flags & LIBSSH2_SESSION_BLOCK_OUTBOUND);
  }
}

void Session::ConnectToServer(OrbitSsh::Credentials creds) {
  credentials_ = std::move(creds);

  notifiers_ = std::nullopt;
  session_ = std::nullopt;
  socket_ = std::nullopt;
  SetState(State::kDisconnected);

  OnEvent();
}

void Session::Disconnect() {
  if (CurrentState() == State::kConnected) {
    SetState(State::kAboutToDisconnect);
  }

  OnEvent();
}

void Session::SetError(std::error_code e) {
  StateMachineHelper::SetError(e);

  notifiers_ = std::nullopt;
  session_ = std::nullopt;
  socket_ = std::nullopt;
}

}  // namespace OrbitSshQt
