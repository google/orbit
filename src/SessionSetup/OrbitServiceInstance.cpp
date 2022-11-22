// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/OrbitServiceInstance.h"

#include <absl/strings/str_format.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QMetaEnum>
#include <QMetaObject>
#include <QProcess>
#include <QtGlobal>
#include <memory>
#include <string>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_session_setup {

constexpr int kWaitTimeInMilliSeconds = 2000;

class OrbitServiceInstanceImpl : public OrbitServiceInstance {
 public:
  explicit OrbitServiceInstanceImpl(const QString& program, const QStringList& arguments);
  ~OrbitServiceInstanceImpl() override;

  [[nodiscard]] bool IsRunning() const override;
  [[nodiscard]] ErrorMessageOr<void> Shutdown() override;

  [[nodiscard]] ErrorMessageOr<void> Start();

 private:
  [[nodiscard]] std::string ReadStateErrorAndOutput();

  QProcess process_;
  QMetaObject::Connection finished_connection_;
};

ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> OrbitServiceInstance::Create(
    const QString& program, const QStringList& arguments) {
  auto instance = std::make_unique<OrbitServiceInstanceImpl>(program, arguments);
  OUTCOME_TRY(instance->Start());

  return std::move(instance);
}

ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> OrbitServiceInstance::CreatePrivileged() {
  const QString orbit_service_path = QCoreApplication::applicationDirPath() + "/OrbitService";
  return OrbitServiceInstance::Create("pkexec", {orbit_service_path});
}

OrbitServiceInstanceImpl::OrbitServiceInstanceImpl(const QString& program,
                                                   const QStringList& arguments) {
  process_.setProgram(program);
  process_.setArguments(arguments);
}

OrbitServiceInstanceImpl::~OrbitServiceInstanceImpl() {
  if (!IsRunning()) return;

  ErrorMessageOr<void> shutdown_result = Shutdown();

  if (shutdown_result.has_error()) {
    ORBIT_ERROR("OrbitService shutdown error: %s", shutdown_result.error().message());
  }
}

bool OrbitServiceInstanceImpl::IsRunning() const {
  return process_.state() == QProcess::ProcessState::Running;
}

ErrorMessageOr<void> OrbitServiceInstanceImpl::Shutdown() {
  if (!IsRunning()) {
    return ErrorMessage{"Unable to shutdown OrbitService, process is not running."};
  }

  QObject::disconnect(finished_connection_);

  // This sends EOF, which signals to OrbitService to shut itself down.
  process_.closeWriteChannel();

  bool wait_finished_result = process_.waitForFinished(kWaitTimeInMilliSeconds);
  if (!wait_finished_result) {
    return ErrorMessage{absl::StrFormat("Shutting down OrbitService timed out after %d ms.",
                                        kWaitTimeInMilliSeconds)};
  }
  return outcome::success();
}

ErrorMessageOr<void> OrbitServiceInstanceImpl::Start() {
  if (IsRunning()) {
    return ErrorMessage{"Unable to start OrbitService, process is already running."};
  }

  process_.start();

  bool wait_started_result = process_.waitForStarted(kWaitTimeInMilliSeconds);
  if (!wait_started_result || !IsRunning()) {
    return ErrorMessage{
        absl::StrFormat("Unable to start OrbitService. Details:\n%s", ReadStateErrorAndOutput())};
  }

  QObject::connect(&process_, &QProcess::errorOccurred, this,
                   [this](const QProcess::ProcessError& error) {
                     emit ErrorOccurred(
                         QString("OrbitService process error occurred, description: %1")
                             .arg(QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error)));
                   });

  finished_connection_ = QObject::connect(
      &process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
      [this](int exit_code, QProcess::ExitStatus exit_status) {
        // If the process crashed, QProcess::errorOccurred is emitted. Hence a crash does not need
        // to be handled here.
        if (exit_status == QProcess::CrashExit) return;

        emit ErrorOccurred(
            QString("OrbitService process ended unexpectedly. exit code: %1").arg(exit_code));
      });

  QObject::connect(&process_, &QProcess::readyReadStandardOutput, this,
                   [this]() { ORBIT_LOG("%s", process_.readAllStandardOutput().toStdString()); });

  QObject::connect(&process_, &QProcess::readyReadStandardError, this,
                   [this]() { ORBIT_LOG("%s", process_.readAllStandardError().toStdString()); });

  return outcome::success();
}

std::string OrbitServiceInstanceImpl::ReadStateErrorAndOutput() {
  return absl::StrFormat("Process state: %s\nProcess error: %s\nstdout: %s\nstderr: %s\n",
                         QMetaEnum::fromType<QProcess::ProcessState>().valueToKey(process_.state()),
                         QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(process_.error()),
                         process_.readAllStandardOutput().toStdString(),
                         process_.readAllStandardError().toStdString());
}

}  // namespace orbit_session_setup
