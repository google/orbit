// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/DeploymentConfigurations.h"

#include <absl/flags/flag.h>

#include <QChar>
#include <QCharRef>
#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>
#include <QString>
#include <optional>

#include "ClientFlags/ClientFlags.h"

static const char* const kSignatureExtension = ".asc";
static const char* const kPackageNameTemplate = "OrbitProfiler-%1.deb";
static const char* const kCollectorSubdirectory = "collector";

namespace {

std::optional<std::string> GetCollectorPath(const QProcessEnvironment& process_environment) {
  constexpr const char* kEnvExecutablePath = "ORBIT_COLLECTOR_EXECUTABLE_PATH";
  if (FLAGS_collector.IsSpecifiedOnCommandLine()) {
    return absl::GetFlag(FLAGS_collector);
  }

  if (process_environment.contains(kEnvExecutablePath)) {
    return process_environment.value(kEnvExecutablePath).toStdString();
  }
  return std::nullopt;
}

std::optional<std::string> GetCollectorRootPassword(
    const QProcessEnvironment& process_environment) {
  constexpr const char* kEnvRootPassword = "ORBIT_COLLECTOR_ROOT_PASSWORD";
  if (FLAGS_collector_root_password.IsSpecifiedOnCommandLine()) {
    return absl::GetFlag(FLAGS_collector_root_password);
  }

  if (process_environment.contains(kEnvRootPassword)) {
    return process_environment.value(kEnvRootPassword).toStdString();
  }

  return std::nullopt;
}

}  // namespace

namespace orbit_session_setup {

SignedDebianPackageDeployment::SignedDebianPackageDeployment() {
  const auto version = []() {
    auto ver = QCoreApplication::applicationVersion();
    if (!ver.isEmpty() && ver[0] == 'v') {
      ver = ver.mid(1);
    }
    return ver;
  }();

  const auto collector_directory =
      QDir{QDir{QCoreApplication::applicationDirPath()}.absoluteFilePath(kCollectorSubdirectory)};

  const auto deb_path =
      collector_directory.absoluteFilePath(QString(kPackageNameTemplate).arg(version));

  path_to_package = deb_path.toStdString();
  path_to_signature = (deb_path + kSignatureExtension).toStdString();
}

DeploymentConfiguration FigureOutDeploymentConfiguration() {
  if (absl::GetFlag(FLAGS_nodeploy)) {
    return NoDeployment{};
  }

  constexpr const char* kEnvPackagePath = "ORBIT_COLLECTOR_PACKAGE_PATH";
  constexpr const char* kEnvSignaturePath = "ORBIT_COLLECTOR_SIGNATURE_PATH";
  constexpr const char* kEnvNoDeployment = "ORBIT_COLLECTOR_NO_DEPLOYMENT";

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  std::optional<std::string> collector_path = GetCollectorPath(env);
  std::optional<std::string> collector_password = GetCollectorRootPassword(env);

  if (collector_path.has_value() && collector_password.has_value()) {
    return orbit_session_setup::BareExecutableAndRootPasswordDeployment{collector_path.value(),
                                                                        collector_password.value()};
  }

  if (env.contains(kEnvPackagePath) && env.contains(kEnvSignaturePath)) {
    return orbit_session_setup::SignedDebianPackageDeployment{
        env.value(kEnvPackagePath).toStdString(), env.value(kEnvSignaturePath).toStdString()};
  }

  if (env.contains(kEnvNoDeployment)) {
    return NoDeployment{};
  }

  return orbit_session_setup::SignedDebianPackageDeployment{};
}

}  // namespace orbit_session_setup
