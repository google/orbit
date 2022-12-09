// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/ExecuteProcess.h"

#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <stdint.h>

#include <QIODevice>
#include <QMetaEnum>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QtGlobal>
#include <memory>
#include <string>

#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"

namespace orbit_qt_utils {

using orbit_base::Future;

Future<ErrorMessageOr<QByteArray>> ExecuteProcess(const QString& program,
                                                  const QStringList& arguments, QObject* parent,
                                                  absl::Duration timeout) {
  auto promise = std::make_shared<orbit_base::Promise<ErrorMessageOr<QByteArray>>>();

  // Create and connect QProcess
  auto* process = new QProcess();
  process->setProgram(program);
  process->setArguments(arguments);

  std::string process_description =
      arguments.empty()
          ? program.toStdString()
          : absl::StrFormat("%s %s", program.toStdString(), arguments.join(" ").toStdString());

  QObject::connect(
      process, &QProcess::errorOccurred, process,
      [promise, process, process_description](QProcess::ProcessError error) {
        process->deleteLater();

        // If the promise already has a result, that means it either the timeout
        // triggered or the parent was destroyed.
        if (promise->HasResult()) return;

        std::string error_message = absl::StrFormat(
            "Error occurred while executing process \"%s\", error: %s,\nstdout:\n%s\nstderr:\n%s\n",
            process_description, QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error),
            process->readAllStandardOutput().toStdString(),
            process->readAllStandardError().toStdString());
        ORBIT_ERROR("%s", error_message);

        promise->SetResult(ErrorMessage{error_message});
      });

  // QProcess::finished is only emitted when the process did actually start. Then the exitStatus
  // can be either NormalExit or CrashExit. When it's CrashExit, the signal errorOccurred will also
  // be emitted, hence it is not handled here.
  QObject::connect(
      process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), process,
      [promise, process, process_description](int exit_code, QProcess::ExitStatus exit_status) {
        if (exit_status == QProcess::CrashExit) return;

        process->deleteLater();

        // If the promise already has a result, that means it either the timeout
        // triggered or the parent was destroyed.
        if (promise->HasResult()) return;

        if (exit_code == 0) {
          promise->SetResult(process->readAllStandardOutput());
          return;
        }

        std::string error_message{absl::StrFormat(
            "Process \"%s\" failed with exit code: %d,\nstdout:\n%s\nstderr:\n%s\n",
            process_description, exit_code, process->readAllStandardOutput().toStdString(),
            process->readAllStandardError().toStdString())};
        ORBIT_ERROR("%s", error_message);
        promise->SetResult(ErrorMessage{error_message});
        return;
      });

  if (parent != nullptr) {
    QObject::connect(
        parent, &QObject::destroyed, process, [process, promise, process_description]() {
          // If the promise already has a result, that means the timeout occurred.
          if (promise->HasResult()) return;

          std::string error_message{
              absl::StrFormat("Process \"%s\" killed because the parent object was destroyed.",
                              process_description)};
          ORBIT_ERROR("%s", error_message);
          promise->SetResult(ErrorMessage{error_message});
          // Killing the process results in errorOccured signal getting emitted. The process is then
          // deleted in the errorOccured signal handler
          process->kill();
        });
  }

  // Create and connect Timer
  // Since timer has process as parent, it will get deleted when process is deleted
  auto* timer = new QTimer(process);
  timer->setSingleShot(true);

  uint64_t timeout_in_ms = timeout / absl::Milliseconds(1);

  // timer has process as target, hence it will only fire if process is not a nullptr.
  QObject::connect(
      timer, &QTimer::timeout, process, [promise, process, process_description, timeout_in_ms]() {
        // If the promise already has a result, that means the parent was already destroyed.
        if (promise->HasResult()) return;

        std::string error_message{absl::StrFormat("Process \"%s\" timed out after %dms",
                                                  process_description, timeout_in_ms)};
        ORBIT_ERROR("%s", error_message);
        promise->SetResult(ErrorMessage{error_message});
        // Killing the process results in errorOccured signal getting emitted. The process is then
        // deleted in the errorOccured signal handler
        process->kill();
      });

  // Start Timer and Process
  timer->start(timeout_in_ms);
  process->start(QIODevice::ReadOnly);

  return promise->GetFuture();
}

}  // namespace orbit_qt_utils