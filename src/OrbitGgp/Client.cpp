// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Client.h"

#include <absl/flags/flag.h>
#include <absl/time/time.h>

#include <QByteArray>
#include <QIODevice>
#include <QPointer>
#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <chrono>
#include <optional>
#include <type_traits>

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/ExecuteProcess.h"
#include "QtUtils/MainThreadExecutorImpl.h"

ABSL_FLAG(uint32_t, ggp_timeout_seconds, 20, "Timeout for Ggp commands in seconds");

namespace orbit_ggp {

using orbit_base::Future;

namespace {

void RunProcessWithTimeout(const QString& program, const QStringList& arguments,
                           std::chrono::milliseconds timeout, QObject* parent,
                           const std::function<void(ErrorMessageOr<QByteArray>)>& callback) {
  const auto process = QPointer<QProcess>{new QProcess{parent}};
  process->setProgram(program);
  process->setArguments(arguments);

  const auto timeout_timer = QPointer<QTimer>{new QTimer{parent}};

  QObject::connect(timeout_timer, &QTimer::timeout, parent,
                   [process, timeout_timer, timeout, callback]() {
                     if ((process != nullptr) && process->state() != QProcess::NotRunning) {
                       std::string error_message =
                           absl::StrFormat("Process request timed out after %dms", timeout.count());
                       ERROR("%s", error_message);
                       callback(ErrorMessage{error_message});
                       if (process != nullptr) {
                         // `process` will also emit an `errorOccured` signal when terminate has
                         // been called, but we don't want that. Hence we have to call
                         // `QObject::disconnect` before-hand.
                         QObject::disconnect(process, nullptr, nullptr, nullptr);
                         process->terminate();
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
                       std::string error_message = absl::StrFormat(
                           "Process failed with error: %s (exit code: "
                           "%d)",
                           process->errorString().toStdString(), exit_code);
                       ERROR("%s", error_message);
                       callback(ErrorMessage{error_message});
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
    std::string error_message =
        absl::StrFormat("Process failed with error: %s", process->errorString().toStdString());
    ERROR("%s", error_message);
    callback(ErrorMessage{error_message});
    process->deleteLater();
  });

  process->start(QIODevice::ReadOnly);
  timeout_timer->start(timeout);
}

}  // namespace

ErrorMessageOr<QPointer<Client>> Client::Create(
    QObject* parent, orbit_qt_utils::MainThreadExecutorImpl* main_thread_executor,
    QString ggp_program, std::chrono::milliseconds timeout) {
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

  return QPointer<Client>(
      new Client{parent, main_thread_executor, std::move(ggp_program), timeout});
}

Future<ErrorMessageOr<QVector<Instance>>> Client::GetInstancesAsync(bool all_reserved,
                                                                    std::optional<Project> project,
                                                                    int retry) {
  QStringList arguments{"instance", "list", "-s"};
  if (all_reserved) {
    arguments.append("--all-reserved");
  }
  if (project != std::nullopt) {
    arguments.append("--project");
    arguments.append(project.value().id);
  }

  Future<ErrorMessageOr<QByteArray>> ggp_call_future =
      orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_));

  return orbit_base::UnwrapFuture(ggp_call_future.Then(
      main_thread_executor_,
      [=, client_ptr = QPointer<QObject>(this)](
          ErrorMessageOr<QByteArray> ggp_call_result) -> Future<ErrorMessageOr<QVector<Instance>>> {
        if (client_ptr == nullptr) return ErrorMessage{"orbit_ggp::Client no longer exists"};

        if (ggp_call_result.has_error()) {
          if (retry > 0) {
            return GetInstancesAsync(all_reserved, project, retry - 1);
          }
          return ggp_call_result.error();
        }
        return Instance::GetListFromJson(ggp_call_result.value());
      }));
}

void Client::GetSshInfoAsync(const Instance& ggp_instance,
                             const std::function<void(ErrorMessageOr<SshInfo>)>& callback,
                             std::optional<Project> project) {
  CHECK(callback);

  QStringList arguments{"ssh", "init", "-s", "--instance", ggp_instance.id};
  if (project != std::nullopt) {
    arguments.append("--project");
    arguments.append(project.value().id);
  }

  RunProcessWithTimeout(ggp_program_, arguments, timeout_, this,
                        [callback](ErrorMessageOr<QByteArray> result) {
                          if (result.has_error()) {
                            callback(result.error());
                          } else {
                            callback(SshInfo::CreateFromJson(result.value()));
                          }
                        });
}

void Client::GetProjectsAsync(
    const std::function<void(ErrorMessageOr<QVector<Project>>)>& callback) {
  QStringList arguments{"project", "list", "-s"};

  RunProcessWithTimeout(ggp_program_, arguments, timeout_, this,
                        [callback](ErrorMessageOr<QByteArray> result) {
                          if (result.has_error()) {
                            callback(result.error());
                            return;
                          }
                          callback(Project::GetListFromJson(result.value()));
                        });
}

std::chrono::milliseconds Client::GetDefaultTimeoutMs() {
  static const uint32_t timeout_seconds = absl::GetFlag(FLAGS_ggp_timeout_seconds);
  static const std::chrono::milliseconds default_timeout_ms(1'000 * timeout_seconds);
  return default_timeout_ms;
}

}  // namespace orbit_ggp
