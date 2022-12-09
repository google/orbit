// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_TASK_H_
#define ORBIT_SSH_QT_TASK_H_

#include <stddef.h>

#include <QMetaType>
#include <QObject>
#include <QPointer>
#include <QString>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Channel.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace orbit_ssh_qt {
namespace details {
enum class TaskState {
  kInitialized,
  kNoChannel,
  kChannelInitialized,
  kStarted,
  kCommandRunning,
  kStopping,
  kSignalEOF,
  kWaitRemoteEOF,
  kSignalChannelClose,
  kWaitChannelClosed,
  kStopped,
  kChannelClosed,
  kError
};

}  // namespace details

/*
  Task is a shell command running on the remote SSH server.

  Currently we don't support changing the default shell, so your command is
  probably evaluated by `bash`.

  It's possible to allocate a pseudo terminal for the command to fake an
  interactive session like it is required for sudo for example. Just pass
  Tty::kYes to the constructor.

  The class supports reading from stdout and writing to stderr. When Stop() is
  called stdin will be closed on the remote side, but there is no support for
  sending signals like SIGINT or SIGKILL. (that's a SSH limitation.)

  When you are interested in the exit code of your command you have to listen
  to the finished signal which has the exit code.
*/
class Task : public StateMachineHelper<Task, details::TaskState> {
  Q_OBJECT
  friend StateMachineHelper;

 public:
  explicit Task(Session* session, std::string command);
  void Start();
  void Stop();

  std::string ReadStdOut();
  std::string ReadStdErr();
  void Write(std::string_view data);

 signals:
  void started();
  void finished(int exit_code);
  void stopped();
  void errorOccurred(std::error_code);
  void readyReadStdOut();
  void readyReadStdErr();
  void bytesWritten(size_t bytes);
  void aboutToShutdown();

 private:
  outcome::result<void> run();
  outcome::result<void> startup();
  outcome::result<void> shutdown();

  using StateMachineHelper::SetError;
  void SetError(std::error_code);

  void HandleSessionShutdown();
  void HandleEagain();

  QPointer<Session> session_;
  std::optional<ScopedConnection> data_event_connection_;
  std::optional<ScopedConnection> about_to_shutdown_connection_;

  std::string command_;
  std::optional<orbit_ssh::Channel> channel_;
  std::string read_std_out_buffer_;
  std::string read_std_err_buffer_;
  std::string write_buffer_;
};

}  // namespace orbit_ssh_qt

Q_DECLARE_METATYPE(size_t);

#endif  // ORBIT_SSH_QT_TASK_H_