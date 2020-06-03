// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/GgpClient.h"

#include <QDebug>
#include <QEventLoop>
#include <QPointer>
#include <QProcess>
#include <QTimer>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpSshInfo.h"

namespace {
constexpr int kDefaultTimeoutInMs = 10'000;

void RunProcessWithTimeout(
    const QString& program, const QStringList& arguments,
    const std::function<void(GgpClient::ResultOrQString<QByteArray>)>& callback,
    int timeout_in_ms = kDefaultTimeoutInMs) {
  const auto process = QPointer{new QProcess{}};
  process->setProgram(program);
  process->setArguments(arguments);

  const auto timeout_timer = QPointer{new QTimer{}};

  QObject::connect(
      timeout_timer, &QTimer::timeout, timeout_timer,
      [process, timeout_timer, callback, timeout_in_ms]() {
        QString error_message =
            QString{"Process request timed out after %1ms"}.arg(timeout_in_ms);
        callback(outcome::failure(error_message));
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
          callback(outcome::failure(
              QString{"Ggp list instances request failed with error: %1 (exit "
                      "code: %2)"}
                  .arg(process->errorString())
                  .arg(exit_code)));
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

GgpClient::ResultOrQString<GgpClient> GgpClient::Create() {
  QProcess ggp_process{};
  ggp_process.setProgram("ggp");
  ggp_process.setArguments({"version"});
  ggp_process.start(QIODevice::ReadOnly);
  ggp_process.waitForFinished();

  if (ggp_process.exitStatus() != QProcess::NormalExit ||
      ggp_process.exitCode() != 0) {
    return QString{
        "Ggp command line process failed with error: %1 (exit code: %2)"}
        .arg(ggp_process.errorString().arg(ggp_process.exitCode()));
  }

  GgpClient client{};
  return client;
}

void GgpClient::GetInstancesAsync(
    const std::function<void(ResultOrQString<QVector<GgpInstance>>)>&
        callback) {
  CHECK(callback);

  number_of_requests_running_++;
  RunProcessWithTimeout(
      "ggp", {"instance", "list", "-s"},
      [callback, this](ResultOrQString<QByteArray> result) {
        number_of_requests_running_--;
        if (!result) {
          callback(result.error());
          return;
        }
        callback(GgpInstance::GetListFromJson(result.value()));
      });
}

void GgpClient::GetSshInformationAsync(
    const GgpInstance& ggpInstance,
    const std::function<void(ResultOrQString<GgpSshInfo>)>& callback) {
  const QStringList arguments{"ssh", "init", "-s", "--instance",
                              ggpInstance.id};
  RunProcessWithTimeout(
      "ggp", arguments, [callback](ResultOrQString<QByteArray> result) {
        if (!result) {
          callback(result.error());
          return;
        }

        std::optional<GgpSshInfo> sshInfo =
            GgpSshInfo::CreateFromJson(result.value());
        if (!sshInfo) {
          callback(QString{"Unable to get ssh info for instance"});
        }
        callback(sshInfo.value());
      });
}