// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QStyleFactory>

#include "App.h"
#include "ApplicationOptions.h"
#include "CrashHandler.h"
#include "CrashOptions.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitStartupWindow.h"
#include "Path.h"
#include "deploymentconfigurations.h"
#include "orbitmainwindow.h"
#include "servicedeploymanager.h"
#include "version.h"

ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");

ABSL_FLAG(bool, enable_debug_menu, false,
          "Enable developer menu in the client's UI");

ABSL_FLAG(std::string, remote, "",
          "Connect to the specified remote on startup");
ABSL_FLAG(uint16_t, asio_port, 44766,
          "The service's Asio tcp_server port (use default value if unsure)");
ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");

// TODO: remove this once we deprecated legacy parameters
static void ParseLegacyCommandLine(int argc, char* argv[],
                                   ApplicationOptions* options) {
  for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
    const char* arg = argv[i];
    if (absl::StartsWith(arg, "gamelet:")) {
      std::cerr << "ERROR: the 'gamelet:<host>:<port>' option is deprecated "
                   "and will be removed soon, please use -remote <host> "
                   "instead."
                << std::endl;

      options->asio_server_address = arg + std::strlen("gamelet:");
    }
  }
}

static outcome::result<void> RunUiInstance(
    QApplication* app,
    OrbitQt::DeploymentConfiguration deployment_configuration,
    ApplicationOptions options) {
  OrbitStartupWindow sw{};
  OUTCOME_TRY(ssh_credentials, sw.Run<OrbitSsh::Credentials>());

  QProgressDialog progress_dialog{};

  const uint16_t asio_port = absl::GetFlag(FLAGS_asio_port);
  const uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  OrbitQt::ServiceDeployManager service_deploy_manager{
      std::move(deployment_configuration), ssh_credentials,
      OrbitQt::ServiceDeployManager::Ports{/* .asio_port = */ asio_port,
                                           /* .grpc_port = */ grpc_port}};
  QObject::connect(&progress_dialog, &QProgressDialog::canceled,
                   &service_deploy_manager,
                   &OrbitQt::ServiceDeployManager::Cancel);
  QObject::connect(&service_deploy_manager,
                   &OrbitQt::ServiceDeployManager::statusMessage,
                   &progress_dialog, &QProgressDialog::setLabelText);
  QObject::connect(
      &service_deploy_manager, &OrbitQt::ServiceDeployManager::statusMessage,
      &service_deploy_manager,
      [](const QString& msg) { LOG("Status message: %s", msg.toStdString()); });

  OUTCOME_TRY(ports, service_deploy_manager.Exec());
  progress_dialog.close();

  options.asio_server_address =
      absl::StrFormat("127.0.0.1:%d", ports.asio_port);
  options.grpc_server_address =
      absl::StrFormat("127.0.0.1:%d", ports.grpc_port);

  OrbitMainWindow w(app, std::move(options));
  w.showMaximized();
  w.PostInit();

  std::optional<std::error_code> error;
  auto error_handler = OrbitSshQt::ScopedConnection{
      QObject::connect(&service_deploy_manager,
                       &OrbitQt::ServiceDeployManager::socketErrorOccurred,
                       &service_deploy_manager, [&](std::error_code e) {
                         error = e;
                         w.close();
                         app->quit();
                       })};

  app->exec();
  GOrbitApp->OnExit();
  if (error) {
    return outcome::failure(error.value());
  } else {
    return outcome::success();
  }
}

static void StyleOrbit(QApplication& app) {
  app.setStyle(QStyleFactory::create("Fusion"));

  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  app.setPalette(darkPalette);
  app.setStyleSheet(
      "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid "
      "white; }");
}

static OrbitQt::DeploymentConfiguration FigureOutDeploymentConfiguration() {
  auto env = QProcessEnvironment::systemEnvironment();

  const char* const kEnvExecutablePath = "ORBIT_COLLECTOR_EXECUTABLE_PATH";
  const char* const kEnvRootPassword = "ORBIT_COLLECTOR_ROOT_PASSWORD";
  const char* const kEnvPackagePath = "ORBIT_COLLECTOR_PACKAGE_PATH";
  const char* const kEnvSignaturePath = "ORBIT_COLLECTOR_SIGNATURE_PATH";
  const char* const kEnvNoDeployment = "ORBIT_COLLECTOR_NO_DEPLOYMENT";

  if (env.contains(kEnvExecutablePath) && env.contains(kEnvRootPassword)) {
    return OrbitQt::BareExecutableAndRootPasswordDeployment{
        env.value(kEnvExecutablePath).toStdString(),
        env.value(kEnvRootPassword).toStdString()};
  } else if (env.contains(kEnvPackagePath) && env.contains(kEnvSignaturePath)) {
    return OrbitQt::SignedDebianPackageDeployment{
        env.value(kEnvPackagePath).toStdString(),
        env.value(kEnvSignaturePath).toStdString()};
  } else if (env.contains(kEnvNoDeployment)) {
    return OrbitQt::NoDeployment{};
  } else {
    return OrbitQt::SignedDebianPackageDeployment{};
  }
}

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("CPU Profiler");
  absl::ParseCommandLine(argc, argv);
#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("Orbit Profiler");
  QCoreApplication::setApplicationVersion(OrbitQt::kVersionString);

  const std::string dump_path = Path::GetDumpPath();
#ifdef _WIN32
  const char* handler_name = "crashpad_handler.exe";
#else
  const char* handler_name = "crashpad_handler";
#endif
  const std::string handler_path = QDir(QCoreApplication::applicationDirPath())
                                       .absoluteFilePath(handler_name)
                                       .toStdString();
  const std::string crash_server_url = CrashServerOptions::GetUrl();
  CrashHandler crash_handler(dump_path, handler_path, crash_server_url);

  ApplicationOptions options;
  ParseLegacyCommandLine(argc, argv, &options);

  StyleOrbit(app);

  const auto deployment_configuration = FigureOutDeploymentConfiguration();

  while (true) {
    const auto result = RunUiInstance(&app, deployment_configuration, options);
    if (result || result.error() == std::errc::interrupted) {
      // It was either a clean shutdown or the user deliberately closed the
      // dialog.
      return 0;
    } else if (result.error() != std::errc::operation_canceled) {
      QMessageBox::critical(
          nullptr,
          QString("%1 %2").arg(QApplication::applicationName(),
                               QApplication::applicationVersion()),
          QString("An error occurred: %1")
              .arg(QString::fromStdString(result.error().message())));
    }
  }
}
