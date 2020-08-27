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

void RunProcessWithTimeout(const QString& program, const QStringList& arguments, QObject* parent,
                           const std::function<void(outcome::result<QByteArray>)>& callback) {
  const auto process = QPointer{new QProcess{parent}};
  process->setProgram(program);
  process->setArguments(arguments);

  const auto timeout_timer = QPointer{new QTimer{parent}};

  QObject::connect(timeout_timer, &QTimer::timeout, parent, [process, timeout_timer, callback]() {
    if (process && !process->waitForFinished(10)) {
      ERROR("Process request timed out after %dms", kDefaultTimeoutInMs);
      callback(Error::kRequestTimedOut);
      if (process) {
        process->terminate();
        process->waitForFinished();
        process->deleteLater();
      }
    }

    timeout_timer->deleteLater();
  });

  QObject::connect(process,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                   parent,
                   [process, timeout_timer, callback](const int exit_code,
                                                      const QProcess::ExitStatus exit_status) {
                     if (timeout_timer) {
                       timeout_timer->stop();
                       timeout_timer->deleteLater();
                     }

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
                   });

  process->start(QIODevice::ReadOnly);
  timeout_timer->start(kDefaultTimeoutInMs);
}

}  // namespace

outcome::result<QPointer<Client>> Client::Create(QObject* parent) {
  QProcess ggp_process{};
  ggp_process.setProgram("ggp");
  ggp_process.setArguments({"version"});
  ggp_process.start(QIODevice::ReadOnly);
  ggp_process.waitForFinished();

  if (ggp_process.exitStatus() != QProcess::NormalExit || ggp_process.exitCode() != 0) {
    ERROR("Ggp command line process failed with error: %s (exit code: %d)",
          ggp_process.errorString().toStdString().c_str(), ggp_process.exitCode());
    return Error::kCouldNotUseGgpCli;
  }

  return QPointer<Client>(new Client{parent});
}

void Client::GetInstancesAsync(
    const std::function<void(outcome::result<QVector<Instance>>)>& callback) {
  CHECK(callback);

  RunProcessWithTimeout("ggp", {"instance", "list", "-s"}, this,
                        [callback](outcome::result<QByteArray> result) {
                          if (!result) {
                            callback(result.error());
                          } else {
                            callback(Instance::GetListFromJson(result.value()));
                          }
                        });
}

void Client::GetSshInfoAsync(const Instance& ggp_instance,
                             const std::function<void(outcome::result<SshInfo>)>& callback) {
  CHECK(callback);

  const QStringList arguments{"ssh", "init", "-s", "--instance", ggp_instance.id};
  RunProcessWithTimeout("ggp", arguments, this, [callback](outcome::result<QByteArray> result) {
    if (!result) {
      callback(result.error());
    } else {
      callback(SshInfo::CreateFromJson(result.value()));
    }
  });
}

}  // namespace OrbitGgp
