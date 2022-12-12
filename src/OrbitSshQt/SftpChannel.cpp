// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/SftpChannel.h"

#include <absl/base/attributes.h>

#include <QtGlobal>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitSshQt/Error.h"

namespace orbit_ssh_qt {

SftpChannel::SftpChannel(Session* session) : session_(session) {
  about_to_shutdown_connection_.emplace(QObject::connect(session_, &Session::aboutToShutdown, this,
                                                         &SftpChannel::HandleSessionShutdown));
}

void SftpChannel::Start() {
  if (CurrentState() == State::kInitialized) {
    SetState(State::kNoChannel);
    OnEvent();
  }
}

void SftpChannel::Stop() {
  if (CurrentState() > State::kInitialized && CurrentState() < State::kStopping) {
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
    case State::kInitialized:
    case State::kNoChannel: {
      OUTCOME_TRY(auto&& sftp, orbit_ssh::Sftp::Init(session_->GetRawSession()));
      sftp_ = std::move(sftp);
      SetState(State::kChannelInitialized);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStarted:
    case State::kChannelInitialized:
      break;
    case State::kStopping:
    case State::kClosingChannel:
    case State::kStopped:
    case State::kError:
      Q_UNREACHABLE();
      break;
  }

  return outcome::success();
}

outcome::result<void> SftpChannel::shutdown() {
  switch (CurrentState()) {
    case State::kInitialized:
    case State::kNoChannel:
    case State::kStarted:
    case State::kChannelInitialized:
      ORBIT_UNREACHABLE();
    case State::kStopping:
    case State::kClosingChannel:
      if (sftp_) {
        OUTCOME_TRY(sftp_->Shutdown());
        sftp_ = std::nullopt;
        SetState(State::kStopped);
      }
      ABSL_FALLTHROUGH_INTENDED;
    case State::kStopped:
      data_event_connection_ = std::nullopt;
      about_to_shutdown_connection_ = std::nullopt;
      break;
    case State::kError:
      ORBIT_UNREACHABLE();
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
  if (session_ != nullptr) {
    session_->HandleEagain();
  }
}

}  // namespace orbit_ssh_qt
