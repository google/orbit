// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Client.h"

#include <QEventLoop>
#include <QPointer>
#include <QProcess>
#include <QTimer>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/SshInfo.h"

namespace OrbitGgp {

namespace {

constexpr int kDefaultTimeoutInMs = 10'000;

void RunProcessWithTimeout(
    const QString& program, const QStringList& arguments,
    const std::function<void(outcome::result<QByteArray>)>& callback,
    int timeout_in_ms = kDefaultTimeoutInMs) {
  const auto process = QPointer{new QProcess{}};
  process->setProgram(program);
  process->setArguments(arguments);

  const auto timeout_timer = QPointer{new QTimer{}};

  QObject::connect(timeout_timer, &QTimer::timeout, timeout_timer,
                   [process, timeout_timer, callback, timeout_in_ms]() {
                     ERROR("Process request timed out after %dms",
                           timeout_in_ms);
                     callback(Error::kRequestTimedOut);
                     process->terminate();
                     process->waitForFinished();
                     if (process != nullptr) process->deleteLater();

                     timeout_timer->deleteLater();
                   });

  QObject::connect(
      process,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      process,
      [process, timeout_timer, callback](
          const int exit_code, const QProcess::ExitStatus exit_status) {
        if (exit_status != QProcess::NormalExit || exit_code != 0) {
          ERROR(
              "Ggp list instances request failed with error: %s (exit code: "
              "%d)",
              process->errorString().toStdString().c_str(), exit_code);
          callback(Error::kGgpListInstancesFailed);
          return;
        }

        callback(outcome::success(process->readAllStandardOutput()));

        process->deleteLater();
        if (timeout_timer != nullptr) {
          timeout_timer->stop();
          timeout_timer->deleteLater();
        }
      });

  process->start(QIODevice::ReadOnly);
  timeout_timer->start(timeout_in_ms);
}

}  // namespace

outcome::result<Client> Client::Create() {
  QProcess ggp_process{};
  ggp_process.setProgram("ggp");
  ggp_process.setArguments({"version"});
  ggp_process.start(QIODevice::ReadOnly);
  ggp_process.waitForFinished();

  if (ggp_process.exitStatus() != QProcess::NormalExit ||
      ggp_process.exitCode() != 0) {
    ERROR("Ggp command line process failed with error: %s (exit code: %d)",
          ggp_process.errorString().toStdString().c_str(),
          ggp_process.exitCode());
    return Error::kCouldNotUseGgpCli;
  }

  Client client{};
  return client;
}

void Client::GetInstancesAsync(
    const std::function<void(outcome::result<QVector<Instance>>)>& callback) {
  CHECK(callback);

  number_of_requests_running_++;
  RunProcessWithTimeout("ggp", {"instance", "list", "-s"},
                        [callback, this](outcome::result<QByteArray> result) {
                          number_of_requests_running_--;
                          if (!result) {
                            callback(result.error());
                            return;
                          }
                          callback(Instance::GetListFromJson(result.value()));
                        });
}

void Client::GetSshInformationAsync(
    const Instance& ggpInstance,
    const std::function<void(outcome::result<SshInfo>)>& callback) {
  const QStringList arguments{"ssh", "init", "-s", "--instance",
                              ggpInstance.id};
  RunProcessWithTimeout("ggp", arguments,
                        [callback](outcome::result<QByteArray> result) {
                          if (!result) {
                            callback(result.error());
                            return;
                          }

                          callback(SshInfo::CreateFromJson(result.value()));
                        });
}

}  // namespace OrbitGgp
