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

const int InstanceRequestTimeoutInMilliseconds = 10'000;

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

  const QByteArray process_result = ggp_process.readAllStandardOutput();
  const QList<QByteArray> tokens = process_result.split(' ');

  if (tokens.size() < 2) {
    return QString{
        "The current version of GGP is not supported by this integration."};
  }

  GgpClient client{};
  client.version_ = tokens.first().toStdString();
  return client;
}

void GgpClient::GetInstancesAsync(
    const std::function<void(ResultOrQString<QVector<GgpInstance>>)>&
        callback) {
  CHECK(callback);

  const auto ggp_process = QPointer{new QProcess{}};
  ggp_process->setProgram("ggp");
  ggp_process->setArguments({"instance", "list", "-s"});

  const auto timeout_timer = QPointer{new QTimer{}};
  QObject::connect(
      timeout_timer, &QTimer::timeout, timeout_timer,
      [ggp_process, timeout_timer, callback, this]() {
        callback(QString("Ggp list instances request timed out after %1 ms.")
                     .arg(InstanceRequestTimeoutInMilliseconds));

        ggp_process->terminate();
        ggp_process->waitForFinished();
        number_of_requests_running_--;

        if (ggp_process != nullptr) ggp_process->deleteLater();

        timeout_timer->deleteLater();
      });

  QObject::connect(
      ggp_process,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      ggp_process,
      [callback, ggp_process, timeout_timer, this](
          const int exit_code, const QProcess::ExitStatus exit_status) {
        if (exit_status != QProcess::NormalExit || exit_code != 0) {
          callback(
              QString{"Ggp list instances request failed with error: %1 (exit "
                      "code: %2)"}
                  .arg(ggp_process->errorString())
                  .arg(exit_code));
          return;
        }

        number_of_requests_running_--;

        const QByteArray jsonData = ggp_process->readAllStandardOutput();
        callback(GgpInstance::GetListFromJson(jsonData));
        ggp_process->deleteLater();

        if (timeout_timer != nullptr) {
          timeout_timer->stop();
          timeout_timer->deleteLater();
        }
      });

  number_of_requests_running_++;
  ggp_process->start(QIODevice::ReadOnly);
  timeout_timer->start(InstanceRequestTimeoutInMilliseconds);
}