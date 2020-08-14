// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/Tunnel.h"

#include <QTimer>

#include "OrbitBase/Logging.h"
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

namespace OrbitSshQt {
Tunnel::Tunnel(Session* session, std::string remote_host, uint16_t remote_port)
    : session_(session), remote_host_(std::move(remote_host)), remote_port_(remote_port) {
  about_to_shutdown_connection_.emplace(
      QObject::connect(session_, &Session::aboutToShutdown, this, &Tunnel::HandleSessionShutdown));
}

void Tunnel::Start() {
  if (CurrentState() == State::kInitial) {
    SetState(State::kNoChannel);
    OnEvent();
  }
}

void Tunnel::Stop() {
  if (CurrentState() == State::kError) {
    return;
  }

  if (CurrentState() < State::kStarted) {
    SetState(State::kDone);
    deleteByEventLoop(this, &local_server_);
    channel_ = std::nullopt;
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
    case State::kInitial:
    case State::kNoChannel: {
      OUTCOME_TRY(channel, OrbitSsh::Channel::OpenTcpIpTunnel(session_->GetRawSession(),
                                                              remote_host_, remote_port_));
      channel_ = std::move(channel);
      SetState(State::kChannelInitialized);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kChannelInitialized: {
      local_server_.emplace();
      const auto result = local_server_->listen();

      if (!result) {
        return Error::kCouldNotListen;
      }

      QObject::connect(&local_server_.value(), &QTcpServer::newConnection, this, [&]() {
        if (!local_socket_) {
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
    case State::kShutdown:
    case State::kFlushing:
    case State::kSendEOF:
    case State::kClosingChannel:
    case State::kWaitRemoteClosed:
    case State::kDone:
    case State::kError:
      UNREACHABLE();
  }
  return outcome::success();
}

outcome::result<void> Tunnel::shutdown() {
  switch (CurrentState()) {
    case State::kInitial:
    case State::kNoChannel:
    case State::kChannelInitialized:
    case State::kStarted:
    case State::kServerListening:
      UNREACHABLE();
    case State::kShutdown:
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
      SetState(State::kDone);
      data_event_connection_ = std::nullopt;
      about_to_shutdown_connection_ = std::nullopt;
      channel_ = std::nullopt;
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kDone:
      break;
    case State::kError:
      UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> Tunnel::readFromChannel() {
  while (true) {
    const size_t kChunkSize = 8192;
    const auto result = channel_->ReadStdOut(kChunkSize);

    if (!result && !OrbitSsh::ShouldITryAgain(result)) {
      return outcome::failure(result.error());
    } else if (!result) {
      // That's the EAGAIN case
      HandleEagain();
      break;
    } else if (result && result.value().empty()) {
      // Empty result means remote socket was closed.
      return Error::kRemoteSocketClosed;
    } else if (result) {
      read_buffer_.append(result.value());
    }
  }

  if (local_socket_ && !read_buffer_.empty()) {
    const auto bytes_written = local_socket_->write(read_buffer_.data(), read_buffer_.size());

    if (bytes_written == -1) {
      SetError(Error::kLocalSocketClosed);
    } else {
      read_buffer_ = read_buffer_.substr(bytes_written);
    }
  }

  return outcome::success();
}

outcome::result<void> Tunnel::writeToChannel() {
  if (!write_buffer_.empty()) {
    OUTCOME_TRY(bytes_written, channel_->Write(write_buffer_));
    write_buffer_ = write_buffer_.substr(bytes_written);
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
  write_buffer_.append(data.toStdString());

  const auto result = writeToChannel();

  if (!result && !OrbitSsh::ShouldITryAgain(result)) {
    SetError(result.error());
    return;
  } else if (!result) {
    HandleEagain();
  }
}

void Tunnel::HandleSessionShutdown() {
  if (CurrentState() >= State::kChannelInitialized && CurrentState() < State::kDone) {
    SetError(Error::kUncleanSessionShutdown);
  }
}

void Tunnel::HandleEagain() {
  if (session_) {
    session_->HandleEagain();
  }
}

}  // namespace OrbitSshQt
