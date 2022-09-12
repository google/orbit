// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Client.h"

#include <absl/flags/flag.h>
#include <absl/strings/match.h>
#include <absl/time/time.h>

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
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
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/ExecuteProcess.h"

ABSL_FLAG(uint32_t, ggp_timeout_seconds, 20, "Timeout for Ggp commands in seconds");

constexpr int kNumberOfRetries = 3;

namespace {
template <typename T>
orbit_base::Future<ErrorMessageOr<T>> RetryTask(
    int retry, std::function<orbit_base::Future<ErrorMessageOr<T>>()> async_task) {
  orbit_base::ImmediateExecutor executor;
  return orbit_base::UnwrapFuture(async_task().Then(
      &executor,
      [retry, async_task](ErrorMessageOr<T> result) -> orbit_base::Future<ErrorMessageOr<T>> {
        if (result.has_value()) return {result.value()};

        if (retry > 0) {
          return RetryTask(retry - 1, async_task);
        }
        return result.error();
      }));
}
}  // namespace

namespace orbit_ggp {

using orbit_base::Future;
using orbit_base::NotFoundOr;

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
  Future<ErrorMessageOr<SshInfo>> GetSshInfoAsync(const QString& instance_id,
                                                  std::optional<Project> project,
                                                  int retry) override;
  Future<ErrorMessageOr<QVector<Project>>> GetProjectsAsync() override;
  Future<ErrorMessageOr<Project>> GetDefaultProjectAsync() override;
  Future<ErrorMessageOr<Instance>> DescribeInstanceAsync(const QString& instance_id) override;
  Future<ErrorMessageOr<Account>> GetDefaultAccountAsync() override;
  Future<ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>> GetSymbolDownloadInfoAsync(
      const SymbolDownloadQuery& symbol_download_query) override;

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

    ORBIT_LOG("%s", error_message);
    ORBIT_LOG("ggp stdout: \"%s\"", QString(ggp_process.readAllStandardOutput()).toStdString());
    return ErrorMessage{error_message};
  }

  return std::make_unique<ClientImpl>(std::move(ggp_program), timeout);
}

Future<ErrorMessageOr<QVector<Instance>>> ClientImpl::GetInstancesAsync(
    InstanceListScope scope, std::optional<Project> project) {
  return GetInstancesAsync(scope, project, kNumberOfRetries);
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

  std::function<Future<ErrorMessageOr<QVector<Instance>>>()> call_ggp_and_create_instance_list =
      [client_ptr = QPointer<ClientImpl>(this),
       arguments = std::move(arguments)]() -> Future<ErrorMessageOr<QVector<Instance>>> {
    if (client_ptr == nullptr) return ErrorMessage{"orbit_ggp::Client no longer exists"};

    orbit_base::ImmediateExecutor executor;
    return orbit_qt_utils::ExecuteProcess(client_ptr->ggp_program_, arguments, client_ptr,
                                          absl::FromChrono(client_ptr->timeout_))
        .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<QVector<Instance>> {
          return Instance::GetListFromJson(json);
        });
  };

  return RetryTask(retry, call_ggp_and_create_instance_list);
}

Future<ErrorMessageOr<SshInfo>> ClientImpl::GetSshInfoAsync(const QString& instance_id,
                                                            std::optional<Project> project) {
  return GetSshInfoAsync(instance_id, std::move(project), kNumberOfRetries);
}

Future<ErrorMessageOr<SshInfo>> ClientImpl::GetSshInfoAsync(const QString& instance_id,
                                                            std::optional<Project> project,
                                                            int retry) {
  QStringList arguments{"ssh", "init", "-s", "--instance", instance_id};
  if (project != std::nullopt) {
    arguments.append("--project");
    arguments.append(project.value().id);
  }

  std::function<Future<ErrorMessageOr<SshInfo>>()> call_ggp_and_create_ssh_info =
      [client_ptr = QPointer<ClientImpl>(this),
       arguments = std::move(arguments)]() -> Future<ErrorMessageOr<SshInfo>> {
    if (client_ptr == nullptr) return {ErrorMessage{"orbit_ggp::Client no longer exists"}};

    orbit_base::ImmediateExecutor executor;
    return orbit_qt_utils::ExecuteProcess(client_ptr->ggp_program_, arguments, client_ptr,
                                          absl::FromChrono(client_ptr->timeout_))
        .ThenIfSuccess(&executor, [](const QByteArray& json) -> ErrorMessageOr<SshInfo> {
          return SshInfo::CreateFromJson(json);
        });
  };
  return RetryTask(retry, std::move(call_ggp_and_create_ssh_info));
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

Future<ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>> ClientImpl::GetSymbolDownloadInfoAsync(
    const SymbolDownloadQuery& symbol_download_query) {
  QStringList arguments{"crash-report",
                        "download-symbols",
                        "-s",
                        "--show-url",
                        "--module",
                        QString("%1/%2")
                            .arg(QString::fromStdString(symbol_download_query.build_id))
                            .arg(QString::fromStdString(symbol_download_query.module_name))};

  orbit_base::ImmediateExecutor executor;
  return orbit_qt_utils::ExecuteProcess(ggp_program_, arguments, this, absl::FromChrono(timeout_))
      .Then(&executor,
            [](ErrorMessageOr<QByteArray> call_ggp_result)
                -> ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>> {
              if (call_ggp_result.has_error()) {
                // When symbols are not found or some other errors occur (e.g., invalid input for
                // the ggp call), orbit_qt_utils::ExecuteProcess returns the error message in the
                // format of "Error occurred while executing process \"<process_description>\",
                // error: <error>,\nstdout:\n<standard_output>\nstderr:\n<standard_error>\n".
                QString error_msg = QString::fromStdString(call_ggp_result.error().message());
                QRegularExpression errorRegex("stderr:\n(.+)\n");
                QRegularExpressionMatch errorMatch = errorRegex.match(QString(error_msg));

                if (!errorMatch.hasMatch()) return ErrorMessage{call_ggp_result.error().message()};

                std::string call_ggp_stderr = errorMatch.captured(1).toStdString();
                // If symbols are not found, the stderr should always contains the following string.
                const std::string kNotFoundString = "some debug symbol files are missing";
                if (absl::StrContains(call_ggp_stderr, kNotFoundString)) {
                  return orbit_base::NotFound("");
                }

                return ErrorMessage{call_ggp_stderr};
              }

              ErrorMessageOr<std::vector<SymbolDownloadInfo>> parse_result =
                  SymbolDownloadInfo::GetListFromJson(call_ggp_result.value());
              if (parse_result.has_error()) return ErrorMessage{parse_result.error().message()};
              // We query a single module for each ggp call. If succeeds, the parse result should
              // always contain a single SymbolDownloadInfo.
              ORBIT_CHECK(parse_result.value().size() == 1);
              return {std::move(parse_result.value().front())};
            });
}

std::chrono::milliseconds GetClientDefaultTimeoutInMs() {
  static const uint32_t timeout_seconds = absl::GetFlag(FLAGS_ggp_timeout_seconds);
  static const std::chrono::milliseconds default_timeout_ms(1'000 * timeout_seconds);
  return default_timeout_ms;
}

}  // namespace orbit_ggp
