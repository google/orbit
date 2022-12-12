// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SFTP_COPY_TO_LOCAL_OPERATION_H_
#define ORBIT_SSH_QT_SFTP_COPY_TO_LOCAL_OPERATION_H_

#include <QFile>
#include <QObject>
#include <QPointer>
#include <QString>
#include <filesystem>
#include <optional>
#include <system_error>

#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "OrbitSsh/SftpFile.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/SftpChannel.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace orbit_ssh_qt {
namespace details {
enum class SftpCopyToLocalOperationState {
  kInitialized,
  kOpenRemoteFile,
  kOpenLocalFile,
  kStarted,  // This is the running state, where the data transfer happens
  kStopping,
  kCloseAndDeletePartialFile,
  kCloseLocalFile,
  kCloseRemoteFile,
  kCloseEventConnections,
  kStopped,
  kError
};
}  // namespace details

/*
  SftpCopyToRemoteOperation represents a file operation in the SSH-SFTP
  subsystem. It needs an established SftpChannel for operation.

  This operation implements remote -> local copying.
*/
class SftpCopyToLocalOperation
    : public StateMachineHelper<SftpCopyToLocalOperation, details::SftpCopyToLocalOperationState> {
  Q_OBJECT
  friend StateMachineHelper;

 public:
  explicit SftpCopyToLocalOperation(Session* session, SftpChannel* channel,
                                    orbit_base::StopToken stop_token);

  void CopyFileToLocal(std::filesystem::path source, std::filesystem::path destination);

  void Stop();

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

  std::filesystem::path source_;
  std::filesystem::path destination_;

  orbit_base::StopToken stop_token_;

  void HandleChannelShutdown();
  void HandleEagain();

  outcome::result<void> run();
  outcome::result<void> startup();
  outcome::result<void> shutdown();

  using StateMachineHelper::SetError;
  void SetError(std::error_code);
};

}  // namespace orbit_ssh_qt
#endif  // ORBIT_SSH_QT_SFTP_COPY_TO_LOCAL_OPERATION_H_
