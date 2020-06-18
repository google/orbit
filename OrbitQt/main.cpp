// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>

#include <QApplication>
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
#include "Error.h"
#include "GlutContext.h"
#include "OrbitBase/Logging.h"
#include "OrbitGgp/Error.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitStartupWindow.h"
#include "Path.h"
#include "deploymentconfigurations.h"
#include "opengldetect.h"
#include "orbitmainwindow.h"
#include "servicedeploymanager.h"
#include "version.h"

ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");

ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

ABSL_FLAG(uint16_t, asio_port, 44766,
          "The service's Asio tcp_server port (use default value if unsure)");
ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");

// TODO: remove this once we deprecated legacy parameters
static void ParseLegacyCommandLine(int argc, char* argv[],
                                   ApplicationOptions* options) {
  for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
    const char* arg = argv[i];
    if (absl::StartsWith(arg, "gamelet:")) {
      ERROR(
          "the 'gamelet:<host>:<port>' option is deprecated and will be "
          "removed soon, please use -remote <host> instead.");

      options->asio_server_address = arg + std::strlen("gamelet:");
    }
  }
}

using ServiceDeployManager = OrbitQt::ServiceDeployManager;
using DeploymentConfiguration = OrbitQt::DeploymentConfiguration;
using OrbitStartupWindow = OrbitQt::OrbitStartupWindow;
using Error = OrbitQt::Error;
using ScopedConnection = OrbitSshQt::ScopedConnection;
using Ports = ServiceDeployManager::Ports;
using SshCredentials = OrbitSsh::Credentials;
using Context = OrbitSsh::Context;

static outcome::result<Ports> DeployOrbitService(
    std::optional<ServiceDeployManager>& service_deploy_manager,
    const DeploymentConfiguration& deployment_configuration, Context* context,
    const SshCredentials& ssh_credentials, const Ports& remote_ports) {
  QProgressDialog progress_dialog{};

  service_deploy_manager.emplace(deployment_configuration, context,
                                 ssh_credentials, remote_ports);
  QObject::connect(&progress_dialog, &QProgressDialog::canceled,
                   &service_deploy_manager.value(),
                   &ServiceDeployManager::Cancel);
  QObject::connect(&service_deploy_manager.value(),
                   &ServiceDeployManager::statusMessage, &progress_dialog,
                   &QProgressDialog::setLabelText);
  QObject::connect(
      &service_deploy_manager.value(), &ServiceDeployManager::statusMessage,
      [](const QString& msg) { LOG("Status message: %s", msg.toStdString()); });

  return service_deploy_manager->Exec();
}

static outcome::result<void> RunUiInstance(
    QApplication* app,
    std::optional<DeploymentConfiguration> deployment_configuration,
    Context* context, ApplicationOptions options) {
  std::optional<OrbitQt::ServiceDeployManager> service_deploy_manager;

  OUTCOME_TRY(result, [&]() -> outcome::result<std::tuple<Ports, QString>> {
    const Ports remote_ports{/*.asio_port =*/absl::GetFlag(FLAGS_asio_port),
                             /*.grpc_port =*/absl::GetFlag(FLAGS_grpc_port)};

    if (deployment_configuration) {
      OrbitStartupWindow sw{};
      OUTCOME_TRY(result, sw.Run<OrbitSsh::Credentials>());

      if (std::holds_alternative<OrbitSsh::Credentials>(result)) {
        // The user chose a remote profiling target.
        OUTCOME_TRY(
            tunnel_ports,
            DeployOrbitService(service_deploy_manager,
                               deployment_configuration.value(), context,
                               std::get<SshCredentials>(result), remote_ports));
        return std::make_tuple(tunnel_ports, QString{});
      } else {
        // The user chose to open a capture.
        return std::make_tuple(remote_ports, std::get<QString>(result));
      }
    } else {
      // When the local flag is present
      return std::make_tuple(remote_ports, QString{});
    }
  }());
  const auto& [ports, capture_path] = result;

  options.asio_server_address =
      absl::StrFormat("127.0.0.1:%d", ports.asio_port);
  options.grpc_server_address =
      absl::StrFormat("127.0.0.1:%d", ports.grpc_port);

  OrbitMainWindow w(app, std::move(options));
  w.showMaximized();
  w.PostInit();

  std::optional<std::error_code> error;
  auto error_handler = [&]() -> ScopedConnection {
    if (service_deploy_manager) {
      return OrbitSshQt::ScopedConnection{QObject::connect(
          &service_deploy_manager.value(),
          &ServiceDeployManager::socketErrorOccurred,
          &service_deploy_manager.value(), [&](std::error_code e) {
            error = e;
            w.close();
            app->quit();
          })};
    } else {
      return ScopedConnection();
    }
  }();

  if (!capture_path.isEmpty()) {
    OUTCOME_TRY(w.OpenCapture(capture_path.toStdString()));
  }

  app->exec();
  GOrbitApp->OnExit();
  if (error) {
    return outcome::failure(error.value());
  } else {
    return outcome::success();
  }
}

static void StyleOrbit(QApplication& app) {
  QFile f(":qdarkstyle/style.qss");
  f.open(QFile::ReadOnly | QFile::Text);
  QTextStream ts(&f);
  app.setStyleSheet(ts.readAll());
}

static std::optional<OrbitQt::DeploymentConfiguration>
FigureOutDeploymentConfiguration() {
  if (absl::GetFlag(FLAGS_local)) {
    return std::nullopt;
  }

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

static void DisplayErrorToUser(const QString& message) {
  QMessageBox::critical(nullptr, QApplication::applicationName(), message);
}

int main(int argc, char* argv[]) {
  const std::string log_file_path = Path::GetLogFilePath();
  InitLogFile(log_file_path);

  absl::SetProgramUsageMessage("CPU Profiler");
  absl::ParseCommandLine(argc, argv);
#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  OrbitGl::GlutContext glut_context{&argc, argv};

  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("Orbit Profiler [BETA]");
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

  const auto open_gl_version = OrbitQt::DetectOpenGlVersion();

  if (!open_gl_version) {
    DisplayErrorToUser(
        "OpenGL support was not found. Please make sure you're not trying to "
        "start Orbit in a remote session and make sure you have a recent "
        "graphics driver installed. Then try again!");
    return -1;
  }

  LOG("Detected OpenGL version: %i.%i", open_gl_version->major,
      open_gl_version->minor);

  if (open_gl_version->major < 2) {
    DisplayErrorToUser(
        QString(
            "The minimum required version of OpenGL is 2.0. But this machine "
            "only supports up to version %1.%2. Please make sure you're not "
            "trying to start Orbit in a remote session and make sure you have "
            "a recent graphics driver installed. Then try again!")
            .arg(open_gl_version->major)
            .arg(open_gl_version->minor));
    return -1;
  }

  auto context = Context::Create();
  if (!context) {
    DisplayErrorToUser(
        QString("An error occurred while initializing ssh: %1")
            .arg(QString::fromStdString(context.error().message())));
    return -1;
  }

  while (true) {
    const auto result = RunUiInstance(&app, deployment_configuration,
                                      &(context.value()), options);
    if (result ||
        result.error() == make_error_code(Error::kUserClosedStartUpWindow) ||
        !deployment_configuration) {
      // It was either a clean shutdown or the deliberately closed the
      // dialog, or we started with the --local flag.
      return 0;
    } else if (result.error() ==
               make_error_code(OrbitGgp::Error::kCouldNotUseGgpCli)) {
      DisplayErrorToUser(QString::fromStdString(result.error().message()));
      return 1;
    } else if (result.error() !=
               make_error_code(Error::kUserCanceledServiceDeployment)) {
      DisplayErrorToUser(
          QString("An error occurred: %1")
              .arg(QString::fromStdString(result.error().message())));
    }
  }
}
