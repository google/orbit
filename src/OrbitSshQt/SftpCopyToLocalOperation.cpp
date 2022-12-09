// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/SftpCopyToLocalOperation.h"

#include <absl/base/attributes.h>
#include <stddef.h>

#include <QIODevice>
#include <string>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/StopToken.h"
#include "OrbitSsh/SftpFile.h"
#include "OrbitSshQt/Error.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace orbit_ssh_qt {

SftpCopyToLocalOperation::SftpCopyToLocalOperation(Session* session, SftpChannel* channel,
                                                   orbit_base::StopToken stop_token)
    : session_(session), channel_(channel), stop_token_(std::move(stop_token)) {
  about_to_shutdown_connection_.emplace(
      QObject::connect(channel_, &SftpChannel::aboutToShutdown, this,
                       &SftpCopyToLocalOperation::HandleChannelShutdown));
}

void SftpCopyToLocalOperation::CopyFileToLocal(std::filesystem::path source,
                                               std::filesystem::path destination) {
  source_ = std::move(source);
  destination_ = std::move(destination);

  SetState(State::kOpenRemoteFile);
  OnEvent();
}

void SftpCopyToLocalOperation::Stop() {
  switch (CurrentState()) {
    case State::kInitialized:
    case State::kOpenRemoteFile: {
      SetState(State::kCloseEventConnections);
      break;
    }
    case State::kOpenLocalFile: {
      SetState(State::kCloseRemoteFile);
      break;
    }
    case State::kStarted:
      SetState(State::kCloseAndDeletePartialFile);
      break;
    case State::kStopping:
    case State::kCloseAndDeletePartialFile:
    case State::kCloseLocalFile:
    case State::kCloseRemoteFile:
    case State::kCloseEventConnections:
    case State::kStopped:
      break;
    case State::kError:
      ORBIT_UNREACHABLE();
  }

  OnEvent();
}

outcome::result<void> SftpCopyToLocalOperation::shutdown() {
  switch (CurrentState()) {
    case State::kInitialized:
    case State::kOpenRemoteFile:
    case State::kOpenLocalFile:
    case State::kStarted:
      ORBIT_UNREACHABLE();
    case State::kStopping:
    case State::kCloseAndDeletePartialFile: {
      local_file_.close();
      bool remove_result = local_file_.remove();
      if (!remove_result) {
        ORBIT_ERROR("Unable to remove partially downloaded file %s",
                    local_file_.fileName().toStdString());
      }
      // The local file is already closed, therefore this jumps directly to state kCloseRemoteFile
      SetState(State::kCloseRemoteFile);
      return shutdown();
    }
    case State::kCloseLocalFile: {
      local_file_.close();
      SetState(State::kCloseRemoteFile);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kCloseRemoteFile: {
      OUTCOME_TRY(sftp_file_->Close());
      SetState(State::kCloseEventConnections);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kCloseEventConnections: {
      about_to_shutdown_connection_ = std::nullopt;
      data_event_connection_ = std::nullopt;
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

outcome::result<void> SftpCopyToLocalOperation::run() {
  ORBIT_CHECK(CurrentState() == State::kStarted);

  constexpr size_t kReadBufferMaxSize = 1 * 1024 * 1024;

  while (true) {
    if (stop_token_.IsStopRequested()) {
      SetState(State::kCloseAndDeletePartialFile);
      break;
    }

    OUTCOME_TRY(auto&& read_buffer, sftp_file_->Read(kReadBufferMaxSize));
    if (read_buffer.empty()) {
      // This is end of file

      SetState(State::kCloseLocalFile);
      break;
    }

    local_file_.write(read_buffer.data(), read_buffer.size());
  }
  return outcome::success();
}

outcome::result<void> SftpCopyToLocalOperation::startup() {
  if (!data_event_connection_) {
    data_event_connection_.emplace(QObject::connect(channel_, &SftpChannel::dataEvent, this,
                                                    &SftpCopyToLocalOperation::OnEvent));
  }

  switch (CurrentState()) {
    case State::kInitialized:
    case State::kOpenRemoteFile: {
      OUTCOME_TRY(auto&& sftp_file,
                  orbit_ssh::SftpFile::Open(session_->GetRawSession(), channel_->GetRawSftp(),
                                            source_.string(), orbit_ssh::FxfFlags::kRead,
                                            0 /* mode - not applicable for kRead */));
      sftp_file_ = std::move(sftp_file);
      SetState(State::kOpenLocalFile);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kOpenLocalFile: {
      local_file_.setFileName(QString::fromStdString(destination_.string()));
      const auto open_result = local_file_.open(QIODevice::WriteOnly);
      if (!open_result) {
        return Error::kCouldNotOpenFile;
      }
      SetState(State::kStarted);
      break;
    }
    case State::kStarted:
    case State::kStopping:
    case State::kCloseAndDeletePartialFile:
    case State::kCloseLocalFile:
    case State::kCloseRemoteFile:
    case State::kCloseEventConnections:
    case State::kStopped:
    case State::kError:
      ORBIT_UNREACHABLE();
  };

  return outcome::success();
}

void SftpCopyToLocalOperation::SetError(std::error_code e) {
  data_event_connection_ = std::nullopt;
  about_to_shutdown_connection_ = std::nullopt;

  StateMachineHelper::SetError(e);

  sftp_file_ = std::nullopt;
  local_file_.close();
}

void SftpCopyToLocalOperation::HandleChannelShutdown() { SetError(Error::kUncleanChannelShutdown); }

void SftpCopyToLocalOperation::HandleEagain() {
  if (session_ != nullptr) {
    session_->HandleEagain();
  }
}

}  // namespace orbit_ssh_qt
