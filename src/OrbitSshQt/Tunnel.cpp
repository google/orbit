// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/Tunnel.h"

#include <absl/base/attributes.h>
#include <stddef.h>

#include <QByteArray>
#include <QHostAddress>
#include <QTimer>
#include <string_view>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"
#include "OrbitSshQt/Error.h"

/**
 * Schedules a task in the currently running event loop.
 *
 * This task checks first whether parent still exists and afterwards destructs
 * the content of opt (which is supposed to be a member of parent.)
 *
 * parent needs to be derived from QObject.
 **/
template <typename P, typename T>
static void deleteByEventLoop(P* parent, std::optional<T>* opt) {
  if (opt) {
    QTimer::singleShot(0, [opt, parent = QPointer<P>(parent)]() {
      if (parent && opt) {
        *opt = std::nullopt;
      }
    });
  }
}

namespace orbit_ssh_qt {
Tunnel::Tunnel(Session* session, std::string remote_host, uint16_t remote_port, QObject* parent)
    : StateMachineHelper(parent),
      session_(session),
      remote_host_(std::move(remote_host)),
      remote_port_(remote_port) {
  about_to_shutdown_connection_.emplace(
      QObject::connect(session_, &Session::aboutToShutdown, this, &Tunnel::HandleSessionShutdown));
}

void Tunnel::Start() {
  if (CurrentState() == State::kInitialized) {
    SetState(State::kNoChannel);
    OnEvent();
  }
}

void Tunnel::Stop() {
  // Tunnel::Stop is currently called from 2 different locations. 1. When the local_socket_ signals
  // a disconnect and 2. from ServiceDeployManager::ShutdownTunnel. If 1. happens before 2.,
  // ShutdownTunnel will wait forever for a stopped signal. This is here solved by emitting
  // stopped() when the Tunnel is already stopped.

  if (CurrentState() == State::kError || CurrentState() == State::kStopped) {
    // TODO (b/208613682) Tunnel::Stop should return a future and not use the stopped signal anymore
    emit stopped();
    return;
  }

  if (CurrentState() < State::kStarted) {
    SetState(State::kStopped);
    deleteByEventLoop(this, &local_server_);
    channel_ = std::nullopt;
    // TODO (b/208613682) Tunnel::Stop should return a future and not use the stopped signal anymore
    emit stopped();
    return;
  }

  if (CurrentState() == State::kServerListening) {
    SetState(State::kFlushing);
    OnEvent();
  }
}

outcome::result<void> Tunnel::startup() {
  if (!data_event_connection_) {
    data_event_connection_.emplace(
        QObject::connect(session_, &Session::dataEvent, this, &Tunnel::OnEvent));
  }

  switch (CurrentState()) {
    case State::kInitialized:
    case State::kNoChannel: {
      OUTCOME_TRY(auto&& channel, orbit_ssh::Channel::OpenTcpIpTunnel(session_->GetRawSession(),
                                                                      remote_host_, remote_port_));
      channel_ = std::move(channel);
      SetState(State::kChannelInitialized);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kChannelInitialized: {
      local_server_.emplace(this);
      const auto result = local_server_->listen(QHostAddress{QHostAddress::LocalHost});

      if (!result) {
        return Error::kCouldNotListen;
      }

      QObject::connect(&local_server_.value(), &QTcpServer::newConnection, this, [&]() {
        if (local_socket_ == nullptr) {
          local_socket_ = local_server_->nextPendingConnection();
          local_server_->pauseAccepting();
          QObject::connect(local_socket_, &QTcpSocket::readyRead, this,
                           &Tunnel::HandleIncomingDataLocalSocket);
          QObject::connect(local_socket_, &QTcpSocket::disconnected, this, [&]() { Stop(); });
        }
      });

      SetState(State::kServerListening);
      emit tunnelOpened(GetListenPort());
      break;
    }
    case State::kStarted:
    case State::kServerListening:
    case State::kStopping:
    case State::kFlushing:
    case State::kSendEOF:
    case State::kWaitRemoteEOF:
    case State::kClosingChannel:
    case State::kWaitRemoteClosed:
    case State::kStopped:
    case State::kError:
      ORBIT_UNREACHABLE();
  }
  return outcome::success();
}

outcome::result<void> Tunnel::shutdown() {
  switch (CurrentState()) {
    case State::kInitialized:
    case State::kNoChannel:
    case State::kChannelInitialized:
    case State::kStarted:
    case State::kServerListening:
      ORBIT_UNREACHABLE();
    case State::kStopping:
    case State::kFlushing: {
      OUTCOME_TRY(writeToChannel());
      SetState(State::kSendEOF);
      // local_server_ might have triggered this shutdown iteration and we can't
      // delete it when it's still somewhere up in the callstack.
      deleteByEventLoop(this, &local_server_);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kSendEOF: {
      OUTCOME_TRY(channel_->SendEOF());
      SetState(State::kWaitRemoteEOF);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kWaitRemoteEOF: {
      OUTCOME_TRY(channel_->WaitRemoteEOF());
      SetState(State::kClosingChannel);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kClosingChannel: {
      OUTCOME_TRY(channel_->Close());
      SetState(State::kWaitRemoteClosed);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kWaitRemoteClosed: {
      OUTCOME_TRY(channel_->WaitClosed());
      SetState(State::kStopped);
      data_event_connection_ = std::nullopt;
      about_to_shutdown_connection_ = std::nullopt;
      channel_ = std::nullopt;
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStopped:
      break;
    case State::kError:
      ORBIT_UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> Tunnel::readFromChannel() {
  ORBIT_SCOPE_FUNCTION;
  while (true) {
    constexpr size_t kChunkSize = 1024 * 1024;
    const auto result = channel_->ReadStdOut(kChunkSize);

    if (!result && !orbit_ssh::ShouldITryAgain(result)) {
      return outcome::failure(result.error());
    } else if (!result) {
      // That's the EAGAIN case
      HandleEagain();
      break;
    } else if (result && result.value().empty()) {
      // Empty result means remote socket was closed.
      return Error::kRemoteSocketClosed;
    } else if (result) {
      ORBIT_UINT64("readFromChannel bytes read", result.value().size());
      read_buffer_.append(result.value());
    }
  }

  if ((local_socket_ != nullptr) && !read_buffer_.empty()) {
    const auto bytes_written = local_socket_->write(read_buffer_.data(), read_buffer_.size());

    if (bytes_written == -1) {
      SetError(Error::kLocalSocketClosed);
    } else {
      ORBIT_CHECK(static_cast<size_t>(bytes_written) <= read_buffer_.size());
      read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + bytes_written);
    }
  }

  return outcome::success();
}

outcome::result<void> Tunnel::writeToChannel() {
  ORBIT_SCOPE_FUNCTION;
  if (!write_buffer_.empty()) {
    const std::string_view buffer_view{write_buffer_.data(), write_buffer_.size()};
    OUTCOME_TRY(auto&& bytes_written, channel_->Write(buffer_view));
    ORBIT_CHECK(static_cast<size_t>(bytes_written) <= write_buffer_.size());
    write_buffer_.erase(write_buffer_.begin(), write_buffer_.begin() + bytes_written);
    ORBIT_UINT64("writeToChannel bytes written", bytes_written);
  }
  return outcome::success();
}

outcome::result<void> Tunnel::run() {
  OUTCOME_TRY(readFromChannel());
  OUTCOME_TRY(writeToChannel());
  return outcome::success();
}

void Tunnel::SetError(std::error_code e) {
  data_event_connection_ = std::nullopt;
  about_to_shutdown_connection_ = std::nullopt;
  StateMachineHelper::SetError(e);

  // local_server_ might have triggered this error and we can't delete it when
  // it's still somewhere up in the callstack.
  deleteByEventLoop(this, &local_server_);
  channel_ = std::nullopt;
}

// local_socket -> write_buffer_
void Tunnel::HandleIncomingDataLocalSocket() {
  const auto data = local_socket_->readAll();
  write_buffer_.insert(write_buffer_.end(), data.begin(), data.end());

  const auto result = writeToChannel();

  if (!result && !orbit_ssh::ShouldITryAgain(result)) {
    SetError(result.error());
    return;
  } else if (!result) {
    HandleEagain();
  }
}

void Tunnel::HandleSessionShutdown() {
  if (CurrentState() >= State::kChannelInitialized && CurrentState() < State::kStopped) {
    SetError(Error::kUncleanSessionShutdown);
  }
}

void Tunnel::HandleEagain() {
  if (session_ != nullptr) {
    session_->HandleEagain();
  }
}

}  // namespace orbit_ssh_qt
