// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/SftpOperation.h"

#include "OrbitBase/Logging.h"

namespace OrbitSshQt {

SftpOperation::SftpOperation(Session* session, SftpChannel* channel)
    : session_(session), channel_(channel) {
  about_to_shutdown_connection_.emplace(
      QObject::connect(channel_, &SftpChannel::aboutToShutdown, this,
                       &SftpOperation::HandleChannelShutdown));
}

void SftpOperation::CopyFileToRemote(std::filesystem::path source,
                                     std::filesystem::path destination,
                                     FileMode destination_mode) {
  source_ = std::move(source);
  destination_ = std::move(destination);
  destination_mode_ = destination_mode;

  SetState(State::kNoOperation);
  OnEvent();
}

outcome::result<void> SftpOperation::shutdown() {
  data_event_connection_ = std::nullopt;
  return outcome::success();
}

outcome::result<void> SftpOperation::run() { return startup(); }

outcome::result<void> SftpOperation::startup() {
  if (!data_event_connection_) {
    data_event_connection_.emplace(QObject::connect(
        channel_, &SftpChannel::dataEvent, this, &SftpOperation::OnEvent));
  }

  switch (CurrentState()) {
    case State::kInitial:
    case State::kNoOperation: {
      local_file_.setFileName(QString::fromStdString(source_.string()));
      const auto open_result = local_file_.open(QIODevice::ReadOnly);
      if (!open_result) {
        return Error::kCouldNotOpenFile;
      }
      SetState(State::kLocalFileOpened);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStarted:
    case State::kLocalFileOpened: {
      OUTCOME_TRY(sftp_file,
                  OrbitSsh::SftpFile::Open(
                      session_->GetRawSession(), channel_->GetRawSftp(),
                      destination_.string(),
                      OrbitSsh::FxfFlags::kWrite | OrbitSsh::FxfFlags::kCreate |
                          OrbitSsh::FxfFlags::kTruncate,
                      static_cast<int>(destination_mode_)));
      sftp_file_ = std::move(sftp_file);
      SetState(State::kRemoteFileOpened);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kRemoteFileOpened: {
      while (true) {
        if (!local_file_.atEnd() && write_buffer_.size() < 32 * 1024) {
          write_buffer_.append(local_file_.read(56 * 1024));
        }

        OUTCOME_TRY(bytes_written,
                    sftp_file_->Write(std::string_view{
                        write_buffer_.constData(),
                        static_cast<size_t>(write_buffer_.size())}));

        write_buffer_.remove(0, bytes_written);

        if (local_file_.atEnd() && write_buffer_.isEmpty()) {
          SetState(State::kRemoteFileWritten);
          break;
        }
      }
    }
    case State::kRemoteFileWritten: {
      OUTCOME_TRY(sftp_file_->Close());
      SetState(State::kRemoteFileClosed);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kRemoteFileClosed: {
      local_file_.close();
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

void SftpOperation::SetError(std::error_code e) {
  data_event_connection_ = std::nullopt;
  about_to_shutdown_connection_ = std::nullopt;

  StateMachineHelper::SetError(e);

  sftp_file_ = std::nullopt;
  write_buffer_.clear();
  local_file_.close();
}

void SftpOperation::HandleChannelShutdown() {
  SetError(Error::kUncleanChannelShutdown);
}

void SftpOperation::HandleEagain() {
  if (session_) {
    session_->HandleEagain();
  }
}

}  // namespace OrbitSshQt
