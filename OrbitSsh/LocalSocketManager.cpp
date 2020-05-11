// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/LocalSocketManager.h"

#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"

namespace OrbitSsh {

LocalSocketManager::LocalSocketManager(std::string address, int port)
    : address_(address), port_(port) {}

ResultType LocalSocketManager::Accept() {
  if (!listen_socket_->CanBeRead()) return ResultType::kAgain;

  std::optional<Socket> new_socket;
  ResultType result = listen_socket_->Accept(&new_socket);

  if (result == ResultType::kSuccess) {
    accepted_socket_ = std::move(new_socket);
  }

  return result;
}

LocalSocketManager::State LocalSocketManager::Tick() {
  switch (state_) {
    case State::kNotInitialized: {
      std::optional<Socket> socket_opt = Socket::Create();
      if (socket_opt.has_value()) {
        listen_socket_ = std::move(socket_opt);
        state_ = State::kInitialized;
      }
      break;
    }
    case State::kInitialized: {
      if (listen_socket_->Bind(address_, port_) == ResultType::kSuccess) {
        state_ = State::kBound;
      }
      break;
    }
    case State::kBound: {
      if (listen_socket_->Listen() == ResultType::kSuccess) {
        state_ = State::kListening;
      }
      break;
    }
    case State::kListening: {
      if (Accept() == ResultType::kSuccess) {
        state_ = State::kRunning;
        LOG("Local Tcp/Ip connection accepted");
      }
      break;
    }
    case State::kRunning: {
      break;
    }
    case State::kShutdownSent: {
      if (accepted_socket_->WaitDisconnect() == ResultType::kSuccess) {
        state_ = State::kRemoteDisconnected;
      }
      break;
    }
    case State::kRemoteDisconnected: {
      accepted_socket_ = std::nullopt;
      state_ = State::kListening;
      break;
    }
  }

  return state_;
}

ResultType LocalSocketManager::Receive(std::string* result) {
  if (state_ != State::kRunning) {
    ERROR("Local socket cant receive because its not running");
    return ResultType::kError;
  }

  if (!accepted_socket_->CanBeRead()) return ResultType::kAgain;

  ResultType receive_result = accepted_socket_->Receive(result);

  if (receive_result == ResultType::kSuccess && result->empty()) {
    state_ = State::kRemoteDisconnected;
    LOG("Local Socket Disconnected");
    return ResultType::kError;
  }

  return receive_result;
}

ResultType LocalSocketManager::SendBlocking(const std::string& text) {
  if (state_ != State::kRunning) {
    ERROR("Local Socket cant send because its not running");
    return ResultType::kError;
  }
  return accepted_socket_->SendBlocking(text);
}

ResultType LocalSocketManager::ForceReconnect() {
  if (state_ != State::kRunning) {
    ERROR("Local Socket cant reconnect because its not running");
    return ResultType::kError;
  }

  ResultType result;
  do {
    result = accepted_socket_->Shutdown();
  } while (result == ResultType::kAgain);

  if (result == ResultType::kError) return ResultType::kError;
  state_ = State::kShutdownSent;
  return ResultType::kSuccess;
}

// This closes the both the listen_socket and the accepted socket gracefully.
// This is depending on the current state_.
ResultType LocalSocketManager::Close() {
  ResultType result;
  switch (state_) {
    // shutdown accepted socket
    case State::kRunning:
      result = accepted_socket_->Shutdown();
      if (result != ResultType::kSuccess) return result;
      state_ = State::kShutdownSent;

    // wait for disconnect
    case State::kShutdownSent:
      result = accepted_socket_->WaitDisconnect();
      if (result != ResultType::kSuccess) return result;
      state_ = State::kRemoteDisconnected;

    // close accepted socket
    case State::kRemoteDisconnected:
      accepted_socket_ = std::nullopt;
      state_ = State::kListening;

    // close listening socket
    case State::kListening:
    case State::kBound:
    case State::kInitialized:
      result = listen_socket_->Shutdown();
      if (result != ResultType::kSuccess) return result;
      listen_socket_ = std::nullopt;
      state_ = State::kNotInitialized;

    // do nothing
    case State::kNotInitialized:
      break;
  }

  state_ = State::kNotInitialized;
  return ResultType::kSuccess;
}
};  // namespace OrbitSsh