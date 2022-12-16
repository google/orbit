// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_TASK_H_
#define ORBIT_SSH_QT_TASK_H_

#include <stddef.h>
#include <stdint.h>

#include <QMetaType>
#include <QObject>
#include <QPointer>
#include <QString>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Typedef.h"
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
  orbit_base::Future<ErrorMessageOr<void>> Start();

  struct ExitCodeTag {};
  using ExitCode = orbit_base::Typedef<ExitCodeTag, int>;
  orbit_base::Future<ErrorMessageOr<ExitCode>> Stop();

  // This is an alternative to Start() and Stop(). Prefer this function over Start/Stop if you don't
  // need to read from or write to stdin/stdout/stderr.
  orbit_base::Future<ErrorMessageOr<ExitCode>> Execute();

  [[nodiscard]] std::string ReadStdOut();
  [[nodiscard]] std::string ReadStdErr();
  orbit_base::Future<ErrorMessageOr<void>> Write(std::string_view data);

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

  struct WritePromise {
    uint64_t completes_when_bytes_written;
    orbit_base::Promise<ErrorMessageOr<void>> promise;

    explicit WritePromise(uint64_t completes_when_bytes_written)
        : completes_when_bytes_written(completes_when_bytes_written) {}
  };
  std::deque<WritePromise> write_promises_;
  uint64_t bytes_written_counter_ = 0;

  std::optional<ExitCode> exit_code_;
};

}  // namespace orbit_ssh_qt

Q_DECLARE_METATYPE(size_t);

#endif  // ORBIT_SSH_QT_TASK_H_