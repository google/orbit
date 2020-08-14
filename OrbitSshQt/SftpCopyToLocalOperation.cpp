// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/SftpCopyToLocalOperation.h"

namespace OrbitSshQt {

SftpCopyToLocalOperation::SftpCopyToLocalOperation(Session* session, SftpChannel* channel)
    : session_(session), channel_(channel) {
  about_to_shutdown_connection_.emplace(
      QObject::connect(channel_, &SftpChannel::aboutToShutdown, this,
                       &SftpCopyToLocalOperation::HandleChannelShutdown));
}

void SftpCopyToLocalOperation::CopyFileToLocal(std::filesystem::path source,
                                               std::filesystem::path destination) {
  source_ = std::move(source);
  destination_ = std::move(destination);

  SetState(State::kNoOperation);
  OnEvent();
}

outcome::result<void> SftpCopyToLocalOperation::shutdown() {
  data_event_connection_ = std::nullopt;
  return outcome::success();
}

outcome::result<void> SftpCopyToLocalOperation::run() { return startup(); }

outcome::result<void> SftpCopyToLocalOperation::startup() {
  if (!data_event_connection_) {
    data_event_connection_.emplace(QObject::connect(channel_, &SftpChannel::dataEvent, this,
                                                    &SftpCopyToLocalOperation::OnEvent));
  }

  switch (CurrentState()) {
    case State::kInitial:
    case State::kNoOperation: {
      OUTCOME_TRY(sftp_file,
                  OrbitSsh::SftpFile::Open(session_->GetRawSession(), channel_->GetRawSftp(),
                                           source_.string(), OrbitSsh::FxfFlags::kRead,
                                           0 /* mode - not applicable for kRead */));
      sftp_file_ = std::move(sftp_file);
      SetState(State::kRemoteFileOpened);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStarted:
    case State::kRemoteFileOpened: {
      local_file_.setFileName(QString::fromStdString(destination_.string()));
      const auto open_result = local_file_.open(QIODevice::WriteOnly);
      if (!open_result) {
        return Error::kCouldNotOpenFile;
      }
      SetState(State::kLocalFileOpened);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kLocalFileOpened: {
      constexpr size_t kReadBufferMaxSize = 1 * 1024 * 1024;

      while (true) {
        OUTCOME_TRY(read_buffer, sftp_file_->Read(kReadBufferMaxSize));
        if (read_buffer.empty()) {
          // This is end of file
          SetState(State::kLocalFileWritten);
          break;
        }

        local_file_.write(read_buffer.data(), read_buffer.size());
      }
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kLocalFileWritten: {
      local_file_.close();
      SetState(State::kLocalFileClosed);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kLocalFileClosed: {
      OUTCOME_TRY(sftp_file_->Close());
      about_to_shutdown_connection_ = std::nullopt;
      SetState(State::kDone);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kShutdown:
    case State::kDone:
      break;
    case State::kError:
      UNREACHABLE();
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
  if (session_) {
    session_->HandleEagain();
  }
}

}  // namespace OrbitSshQt
