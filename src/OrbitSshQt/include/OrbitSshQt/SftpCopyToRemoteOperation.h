// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SFTP_COPY_TO_REMOTE_OPERATION_H_
#define ORBIT_SSH_QT_SFTP_COPY_TO_REMOTE_OPERATION_H_

#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QPointer>
#include <QString>
#include <filesystem>
#include <optional>
#include <system_error>

#include "OrbitBase/Result.h"
#include "OrbitSsh/SftpFile.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace orbit_ssh_qt {
namespace details {
enum class SftpCopyToRemoteOperationState {
  kInitialized,
  kNoOperation,
  kStarted,
  kLocalFileOpened,
  kRemoteFileOpened,
  kRemoteFileWritten,
  kRemoteFileClosed,
  kStopping,
  kStopped,
  kError
};
}  // namespace details

/*
  SftpCopyToRemoteOperation represents a file operation in the SSH-SFTP
  subsystem. It needs an established SftpChannel for operation.

  This operation implements local -> remote copying, for remote to local use
  SftpCopyToLocalOperation.
*/
class SftpCopyToRemoteOperation
    : public StateMachineHelper<SftpCopyToRemoteOperation,
                                details::SftpCopyToRemoteOperationState> {
  Q_OBJECT
  friend StateMachineHelper;

 public:
  enum class FileMode : int { kUserWritable = 0644, kUserWritableAllExecutable = 0755 };

  explicit SftpCopyToRemoteOperation(Session* session, SftpChannel* channel);

  void CopyFileToRemote(std::filesystem::path source, std::filesystem::path destination,
                        FileMode destination_mode);

 signals:
  void started();
  void stopped();
  void aboutToShutdown();
  void errorOccurred(std::error_code);

 private:
  QPointer<Session> session_;

  std::optional<ScopedConnection> data_event_connection_;
  std::optional<ScopedConnection> about_to_shutdown_connection_;

  QPointer<SftpChannel> channel_;
  std::optional<orbit_ssh::SftpFile> sftp_file_;
  QFile local_file_;
  QByteArray write_buffer_;

  std::filesystem::path source_;
  std::filesystem::path destination_;
  FileMode destination_mode_;

  void HandleChannelShutdown();
  void HandleEagain();

  outcome::result<void> run();
  outcome::result<void> startup();
  outcome::result<void> shutdown();

  using StateMachineHelper::SetError;
  void SetError(std::error_code);
};

}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_SFTP_COPY_TO_REMOTE_OPERATION_H_
