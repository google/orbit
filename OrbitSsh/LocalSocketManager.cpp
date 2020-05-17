// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/LocalSocketManager.h"

#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"

namespace OrbitSsh {

LocalSocketManager::LocalSocketManager(std::string address, int port)
    : address_(address), port_(port) {}

outcome::result<void> LocalSocketManager::Accept() {
  if (!listen_socket_->CanBeRead()) return Error::kEagain;

  OUTCOME_TRY(accepted_socket, listen_socket_->Accept());
  accepted_socket_ = std::move(accepted_socket);
  return outcome::success();
}

outcome::result<void> LocalSocketManager::Connect() {
  switch (state_) {
    case State::kNotInitialized: {
      OUTCOME_TRY(listen_socket, Socket::Create());
      listen_socket_ = std::move(listen_socket);
      state_ = State::kInitialized;
    }
    case State::kInitialized: {
      OUTCOME_TRY(listen_socket_->Bind(address_, port_));
      state_ = State::kBound;
    }
    case State::kBound: {
      OUTCOME_TRY(listen_socket_->Listen());
      state_ = State::kListening;
    }
    case State::kListening: {
      OUTCOME_TRY(Accept());
      state_ = State::kRunning;
    }
    case State::kRunning: {
      break;
    }
    case State::kShutdownSent: {
      OUTCOME_TRY(accepted_socket_->WaitDisconnect());
      state_ = State::kRemoteDisconnected;
    }
    case State::kRemoteDisconnected: {
      accepted_socket_ = std::nullopt;
      state_ = State::kListening;
      break;
    }
  }

  return outcome::success();
}

outcome::result<std::string> LocalSocketManager::Receive() {
  CHECK(state_ == State::kRunning);

  if (!accepted_socket_->CanBeRead()) {
    return Error::kEagain;
  }

  OUTCOME_TRY(data, accepted_socket_->Receive());

  if (data.empty()) {
    state_ = State::kRemoteDisconnected;
  }

  return std::move(data);
}

outcome::result<void> LocalSocketManager::SendBlocking(std::string_view data) {
  CHECK(state_ == State::kRunning);
  return accepted_socket_->SendBlocking(data);
}

outcome::result<void> LocalSocketManager::ForceReconnect() {
  CHECK(state_ == State::kRunning);

  while (true) {
    const auto result = accepted_socket_->Shutdown();
    if (result) {
      break;
    }
    if (!result && !shouldITryAgain(result)) {
      return result;
    }
  }

  state_ = State::kShutdownSent;
  return outcome::success();
}

// This closes the both the listen_socket and the accepted socket gracefully.
// This is depending on the current state_.
outcome::result<void> LocalSocketManager::Close() {
  switch (state_) {
    // shutdown accepted socket
    case State::kRemoteDisconnected:
    case State::kRunning: {
      OUTCOME_TRY(accepted_socket_->Shutdown());
      state_ = State::kShutdownSent;
    }

    // wait for disconnect
    case State::kShutdownSent: {
      OUTCOME_TRY(accepted_socket_->WaitDisconnect());
      accepted_socket_ = std::nullopt;
      state_ = State::kListening;
    }

    // close listening socket
    case State::kListening:
    case State::kBound:
    case State::kInitialized: {
      OUTCOME_TRY(listen_socket_->Shutdown());
      listen_socket_ = std::nullopt;
      state_ = State::kNotInitialized;
    }

    // do nothing
    case State::kNotInitialized:
      break;
  }

  return outcome::success();
}
};  // namespace OrbitSsh
