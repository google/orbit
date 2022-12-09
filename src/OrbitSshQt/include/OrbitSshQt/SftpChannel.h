// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SFTP_CHANNEL_H_
#define ORBIT_SSH_QT_SFTP_CHANNEL_H_

#include <QObject>
#include <QPointer>
#include <QString>
#include <optional>
#include <system_error>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Sftp.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace orbit_ssh_qt {
namespace details {
enum class SftpChannelState {
  kInitialized,
  kNoChannel,
  kStarted,
  kChannelInitialized,
  kStopping,
  kClosingChannel,
  kStopped,
  kError
};
}  // namespace details

/*
  SftpChannel is a standard SSH channel with the SFTP subsystem initialized.

  That's a requirement for issuing SFTP commands to the remote side.

  The user needs to wait for the started() signal before they can start an
  SftpCopyToRemoteOperation. The user needs to keep the channel alive as long as
  SftpOperations are still running.
*/
class SftpChannel : public StateMachineHelper<SftpChannel, details::SftpChannelState> {
  Q_OBJECT
  friend StateMachineHelper;

 public:
  explicit SftpChannel(Session* session);

  void Start();
  void Stop();

  orbit_ssh::Sftp* GetRawSftp() { return sftp_ ? &sftp_.value() : nullptr; }

 signals:
  void aboutToShutdown();
  void started();
  void stopped();
  void dataEvent();
  void errorOccurred(std::error_code);

 private:
  QPointer<Session> session_;
  std::optional<orbit_ssh::Sftp> sftp_;

  std::optional<ScopedConnection> data_event_connection_;
  std::optional<ScopedConnection> about_to_shutdown_connection_;

  void HandleSessionShutdown();
  void HandleEagain();

  using StateMachineHelper::SetError;
  void SetError(std::error_code);

  outcome::result<void> startup();
  outcome::result<void> shutdown();
  outcome::result<void> run();
};

}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_SFTP_CHANNEL_H_