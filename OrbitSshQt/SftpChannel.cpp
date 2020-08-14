// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/SftpChannel.h"

#include "OrbitBase/Logging.h"

namespace OrbitSshQt {

SftpChannel::SftpChannel(Session* session) : session_(session) {
  about_to_shutdown_connection_.emplace(QObject::connect(session_, &Session::aboutToShutdown, this,
                                                         &SftpChannel::HandleSessionShutdown));
}

void SftpChannel::Start() {
  if (CurrentState() == State::kInitial) {
    SetState(State::kNoChannel);
    OnEvent();
  }
}

void SftpChannel::Stop() {
  if (CurrentState() > State::kInitial && CurrentState() < State::kShutdown) {
    SetState(State::kClosingChannel);
    OnEvent();
  }
}

outcome::result<void> SftpChannel::startup() {
  if (!data_event_connection_) {
    data_event_connection_.emplace(
        QObject::connect(session_, &Session::dataEvent, this, &SftpChannel::OnEvent));
  }

  switch (CurrentState()) {
    case State::kInitial:
    case State::kNoChannel: {
      OUTCOME_TRY(sftp, OrbitSsh::Sftp::Init(session_->GetRawSession()));
      sftp_ = std::move(sftp);
      SetState(State::kChannelInitialized);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStarted:
    case State::kChannelInitialized:
      break;
    case State::kShutdown:
    case State::kClosingChannel:
    case State::kDone:
    case State::kError:
      Q_UNREACHABLE();
      break;
  }

  return outcome::success();
}

outcome::result<void> SftpChannel::shutdown() {
  switch (CurrentState()) {
    case State::kInitial:
    case State::kNoChannel:
    case State::kStarted:
    case State::kChannelInitialized:
      UNREACHABLE();
    case State::kShutdown:
    case State::kClosingChannel:
      if (sftp_) {
        OUTCOME_TRY(sftp_->Shutdown());
        sftp_ = std::nullopt;
        SetState(State::kDone);
      }
      ABSL_FALLTHROUGH_INTENDED;
    case State::kDone:
      data_event_connection_ = std::nullopt;
      about_to_shutdown_connection_ = std::nullopt;
      break;
    case State::kError:
      UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> SftpChannel::run() {
  emit dataEvent();
  return outcome::success();
}

void SftpChannel::SetError(std::error_code e) {
  data_event_connection_ = std::nullopt;
  about_to_shutdown_connection_ = std::nullopt;
  StateMachineHelper::SetError(e);
  sftp_ = std::nullopt;
}

void SftpChannel::HandleSessionShutdown() {
  emit aboutToShutdown();
  SetError(Error::kUncleanSessionShutdown);
}

void SftpChannel::HandleEagain() {
  if (session_) {
    session_->HandleEagain();
  }
}

}  // namespace OrbitSshQt
