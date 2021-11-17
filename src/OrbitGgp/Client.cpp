// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Client.h"

#include <absl/flags/flag.h>
#include <absl/time/time.h>

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <chrono>
#include <memory>
#include <optional>
#include <type_traits>

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/ExecuteProcess.h"

ABSL_FLAG(uint32_t, ggp_timeout_seconds, 20, "Timeout for Ggp commands in seconds");

namespace orbit_ggp {

using orbit_base::Future;

class ClientImpl : public Client, public QObject {
 public:
  explicit ClientImpl(QString ggp_program, std::chrono::milliseconds timeout)
      : ggp_program_(std::move(ggp_program)), timeout_(timeout) {}

  Future<ErrorMessageOr<QVector<Instance>>> GetInstancesAsync(
      InstanceListScope scope, std::optional<Project> project) override;
  Future<ErrorMessageOr<QVector<Instance>>> GetInstancesAsync(InstanceListScope scope,
                                                              std::optional<Project> project,
                                                              int retry) override;
  Future<ErrorMessageOr<SshInfo>> GetSshInfoAsync(const QString& instance_id,
                                                  std::optional<Project> project) override;
  Future<ErrorMessageOr<QVector<Project>>> GetProjectsAsync() override;
  Future<ErrorMessageOr<Project>> GetDefaultProjectAsync() override;
  Future<ErrorMessageOr<Instance>> DescribeInstanceAsync(const QString& instance_id) override;
  Future<ErrorMessageOr<Account>> GetDefaultAccountAsync() override;

 private:
  const QString ggp_program_;
  const std::chrono::milliseconds timeout_;
};

ErrorMessageOr<std::unique_ptr<Client>> CreateClient(QString ggp_program,
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

  return std::make_unique<ClientImpl>(std::move(ggp_program), timeout);
}

Future<ErrorMessageOr<QVector<Instance>>> ClientImpl::GetInstancesAsync(
    InstanceListScope scope, std::optional<Project> project) {
  return GetInstancesAsync(scope, project, 3);
}

Future<ErrorMessageOr<QVector<Instance>>> ClientImpl::GetInstancesAsync(
    InstanceListScope scope, std::optional<Project> project, int retry) {
  QStringList arguments{"instance", "list", "-s"};
  if (scope == InstanceListScope::kAllReservedInstances) {
    arguments.append("--all-reserved");
  }
  if (project != std::nullopt) {
    arguments.append("--project");
    arguments.append(project.value().id);
  }

  Future<ErrorMessageOr<QByteArray>> ggp_call_future =
      orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_));

  orbit_base::ImmediateExecutor executor;
  return orbit_base::UnwrapFuture(ggp_call_future.Then(
      &executor,
      [=,
       client_ptr = QPointer<ClientImpl>(this)](ErrorMessageOr<QByteArray> ggp_call_result) mutable
      -> Future<ErrorMessageOr<QVector<Instance>>> {
        if (client_ptr == nullptr) return ErrorMessage{"orbit_ggp::Client no longer exists"};

        if (ggp_call_result.has_error()) {
          if (retry > 0) {
            return client_ptr->GetInstancesAsync(scope, project, retry - 1);
          }
          return ggp_call_result.error();
        }
        return Instance::GetListFromJson(ggp_call_result.value());
      }));
}

Future<ErrorMessageOr<SshInfo>> ClientImpl::GetSshInfoAsync(const QString& instance_id,
                                                            std::optional<Project> project) {
  QStringList arguments{"ssh", "init", "-s", "--instance", instance_id};
  if (project != std::nullopt) {
    arguments.append("--project");
    arguments.append(project.value().id);
  }

  orbit_base::ImmediateExecutor executor;
  return orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_))
      .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<SshInfo> {
        return SshInfo::CreateFromJson(json);
      });
}

Future<ErrorMessageOr<QVector<Project>>> ClientImpl::GetProjectsAsync() {
  QStringList arguments{"project", "list", "-s"};

  orbit_base::ImmediateExecutor executor;
  return orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_))
      .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<QVector<Project>> {
        return Project::GetListFromJson(json);
      });
}

Future<ErrorMessageOr<Project>> ClientImpl::GetDefaultProjectAsync() {
  QStringList arguments{"config", "describe", "-s"};
  orbit_base::ImmediateExecutor executor;
  return orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_))
      .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<Project> {
        return Project::GetDefaultProjectFromJson(json);
      });
}

Future<ErrorMessageOr<Instance>> ClientImpl::DescribeInstanceAsync(const QString& instance_id) {
  QStringList arguments{"instance", "describe", instance_id, "-s"};

  orbit_base::ImmediateExecutor executor;
  return orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_))
      .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<Instance> {
        return Instance::CreateFromJson(json);
      });
}

Future<ErrorMessageOr<Account>> ClientImpl::GetDefaultAccountAsync() {
  QStringList arguments{"auth", "list", "-s"};

  orbit_base::ImmediateExecutor executor;
  return orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_))
      .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<Account> {
        return Account::GetDefaultAccountFromJson(json);
      });
}

std::chrono::milliseconds GetClientDefaultTimeoutInMs() {
  static const uint32_t timeout_seconds = absl::GetFlag(FLAGS_ggp_timeout_seconds);
  static const std::chrono::milliseconds default_timeout_ms(1'000 * timeout_seconds);
  return default_timeout_ms;
}

}  // namespace orbit_ggp
