// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/Session.h"

#include <absl/base/attributes.h>
#include <libssh2.h>

#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Session.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace orbit_ssh_qt {

outcome::result<void> Session::startup() {
  switch (CurrentState()) {
    case State::kInitialized:
    case State::kDisconnected: {
      OUTCOME_TRY(auto&& socket, orbit_ssh::Socket::Create());
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
      OUTCOME_TRY(auto&& session, orbit_ssh::Session::Create(context_));
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
      while (true) {
        outcome::result<void> result = session_->Authenticate(
            credentials_.user, credentials_.key_paths.at(next_credential_key_index_));
        if (!result.has_error()) break;
        if (result.has_error() && orbit_ssh::ShouldITryAgain(result)) return result;
        if (++next_credential_key_index_ == credentials_.key_paths.size()) return result;
      }
      SetState(State::kConnected);
      break;
    }
    case State::kStarted:
    case State::kConnected:
    case State::kStopping:
    case State::kAboutToDisconnect:
    case State::kStopped:
    case State::kError:
      ORBIT_UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> Session::shutdown() {
  switch (CurrentState()) {
    case State::kInitialized:
    case State::kDisconnected:
    case State::kSocketCreated:
    case State::kSocketConnected:
    case State::kSessionCreated:
    case State::kHandshaked:
    case State::kMatchedKnownHosts:
    case State::kStarted:
    case State::kConnected:
      ORBIT_UNREACHABLE();
    case State::kStopping:
    case State::kAboutToDisconnect: {
      OUTCOME_TRY(session_->Disconnect());
      notifiers_ = std::nullopt;
      socket_ = std::nullopt;
      session_ = std::nullopt;
      SetState(State::kStopped);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStopped:
      break;
    case State::kError:
      ORBIT_UNREACHABLE();
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

    notifiers_->read.setEnabled((flags & LIBSSH2_SESSION_BLOCK_INBOUND) != 0);
    notifiers_->write.setEnabled((flags & LIBSSH2_SESSION_BLOCK_OUTBOUND) != 0);
  }
}

orbit_base::Future<ErrorMessageOr<void>> Session::ConnectToServer(orbit_ssh::Credentials creds) {
  if (creds.key_paths.empty()) {
    return ErrorMessage{"No private key path was provided."};
  }
  credentials_ = std::move(creds);

  notifiers_ = std::nullopt;
  session_ = std::nullopt;
  socket_ = std::nullopt;
  SetState(State::kDisconnected);

  OnEvent();
  return GetStartedFuture();
}

orbit_base::Future<ErrorMessageOr<void>> Session::Disconnect() {
  if (CurrentState() == State::kConnected) {
    SetState(State::kAboutToDisconnect);
    OnEvent();
  }

  return GetStoppedFuture();
}

void Session::SetError(std::error_code e) {
  StateMachineHelper::SetError(e);

  notifiers_ = std::nullopt;
  session_ = std::nullopt;
  socket_ = std::nullopt;
}

void Session::SetState(details::SessionState state) {
  auto previous_state = CurrentState();
  StateMachineHelper::SetState(state);
  if (previous_state != state && state == State::kMatchedKnownHosts) {
    next_credential_key_index_ = 0;
  }
}

}  // namespace orbit_ssh_qt
