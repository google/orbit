// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Client.h"

#include <QByteArray>
#include <QIODevice>
#include <QPointer>
#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <chrono>
#include <type_traits>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/SshInfo.h"

namespace orbit_ggp {

namespace {

constexpr int kAdditionalSynchronousTimeoutInMs = 20;

void RunProcessWithTimeout(const QString& program, const QStringList& arguments,
                           std::chrono::milliseconds timeout, QObject* parent,
                           const std::function<void(outcome::result<QByteArray>)>& callback) {
  const auto process = QPointer{new QProcess{parent}};
  process->setProgram(program);
  process->setArguments(arguments);

  const auto timeout_timer = QPointer{new QTimer{parent}};

  QObject::connect(timeout_timer, &QTimer::timeout, parent,
                   [process, timeout_timer, timeout, callback]() {
                     if (process && !process->waitForFinished(kAdditionalSynchronousTimeoutInMs)) {
                       ERROR("Process request timed out after %dms", timeout.count());
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
                     if (timeout_timer != nullptr) {
                       timeout_timer->stop();
                       timeout_timer->deleteLater();
                     }

                     if (exit_status != QProcess::NormalExit || exit_code != 0) {
                       ERROR(
                           "Ggp list instances request failed with error: %s (exit code: "
                           "%d)",
                           process->errorString().toStdString(), exit_code);
                       callback(Error::kGgpListInstancesFailed);
                       return;
                     }

                     callback(outcome::success(process->readAllStandardOutput()));

                     process->deleteLater();
                   });

  QObject::connect(process, &QProcess::errorOccurred, parent, [timeout_timer, process, callback]() {
    if (timeout_timer != nullptr) {
      timeout_timer->stop();
      timeout_timer->deleteLater();
    }
    ERROR("Ggp list instances request failed with error: %s", process->errorString().toStdString());
    callback(Error::kGgpListInstancesFailed);
    process->deleteLater();
  });

  process->start(QIODevice::ReadOnly);
  timeout_timer->start(timeout);
}

}  // namespace

ErrorMessageOr<QPointer<Client>> Client::Create(QObject* parent, QString ggp_program,
                                                std::chrono::milliseconds timeout) {
  QProcess ggp_process{};
  ggp_process.setProgram(ggp_program);
  ggp_process.setArguments({"version"});

  ggp_process.start(QIODevice::ReadOnly);
  ggp_process.waitForFinished();

  // QProcess::unknownError is the default, or in other words, if no error happened this will be the
  // return value
  if (ggp_process.exitStatus() != QProcess::NormalExit || ggp_process.exitCode() != 0 ||
      ggp_process.error() != QProcess::UnknownError) {
    std::string error_message =
        absl::StrFormat("Ggp command line process failed with error: %s (exit code: %d)",
                        ggp_process.errorString().toStdString(), ggp_process.exitCode());

    std::string ggp_stderr = QString(ggp_process.readAllStandardError()).toStdString();
    if (!ggp_stderr.empty()) {
      error_message.append(absl::StrFormat(", ggp error message: \"%s\"", ggp_stderr));
    }

    LOG("%s", error_message);
    LOG("ggp stdout: \"%s\"", QString(ggp_process.readAllStandardOutput()).toStdString());
    return ErrorMessage{error_message};
  }

  return QPointer<Client>(new Client{parent, std::move(ggp_program), timeout});
}

void Client::GetInstancesAsync(
    const std::function<void(outcome::result<QVector<Instance>>)>& callback, int retry) {
  CHECK(callback);

  RunProcessWithTimeout(ggp_program_, {"instance", "list", "-s"}, timeout_, this,
                        [this, callback, retry](outcome::result<QByteArray> result) {
                          if (!result) {
                            if (retry < 1) {
                              callback(result.error());
                            } else {
                              GetInstancesAsync(callback, retry - 1);
                            }
                          } else {
                            callback(Instance::GetListFromJson(result.value()));
                          }
                        });
}

void Client::GetSshInfoAsync(const Instance& ggp_instance,
                             const std::function<void(outcome::result<SshInfo>)>& callback) {
  CHECK(callback);

  const QStringList arguments{"ssh", "init", "-s", "--instance", ggp_instance.id};
  RunProcessWithTimeout(ggp_program_, arguments, timeout_, this,
                        [callback](outcome::result<QByteArray> result) {
                          if (!result) {
                            callback(result.error());
                          } else {
                            callback(SshInfo::CreateFromJson(result.value()));
                          }
                        });
}

}  // namespace orbit_ggp
