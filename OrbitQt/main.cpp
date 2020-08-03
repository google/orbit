// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/flags/usage_config.h>

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QStyleFactory>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include "App.h"
#include "ApplicationOptions.h"
#include "Error.h"
#include "OrbitBase/Logging.h"
#include "OrbitGgp/Error.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/Session.h"
#include "OrbitStartupWindow.h"
#include "OrbitVersion.h"
#include "Path.h"
#include "deploymentconfigurations.h"
#include "opengldetect.h"
#include "orbitmainwindow.h"
#include "servicedeploymanager.h"

#ifdef ORBIT_CRASH_HANDLING
#include "CrashHandler.h"
#include "CrashOptions.h"
#endif

ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");

ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Frequency of callstack sampling in samples per second");

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_FLAG(bool, frame_pointer_unwinding, false,
          "Use frame pointers for unwinding");

using ServiceDeployManager = OrbitQt::ServiceDeployManager;
using DeploymentConfiguration = OrbitQt::DeploymentConfiguration;
using OrbitStartupWindow = OrbitQt::OrbitStartupWindow;
using Error = OrbitQt::Error;
using ScopedConnection = OrbitSshQt::ScopedConnection;
using GrpcPort = ServiceDeployManager::GrpcPort;
using SshCredentials = OrbitSsh::Credentials;
using Context = OrbitSsh::Context;

static outcome::result<GrpcPort> DeployOrbitService(
    std::optional<ServiceDeployManager>& service_deploy_manager,
    const DeploymentConfiguration& deployment_configuration, Context* context,
    const SshCredentials& ssh_credentials, const GrpcPort& remote_ports) {
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
    Context* context) {
  std::optional<OrbitQt::ServiceDeployManager> service_deploy_manager;

  OUTCOME_TRY(result, [&]() -> outcome::result<std::tuple<GrpcPort, QString>> {
    const GrpcPort remote_ports{/*.grpc_port =*/absl::GetFlag(FLAGS_grpc_port)};

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

  ApplicationOptions options;
  options.grpc_server_address =
      absl::StrFormat("127.0.0.1:%d", ports.grpc_port);

  OrbitMainWindow w(app, std::move(options));
  // "resize" is required to make "showMaximized" work properly.
  w.resize(1280, 720);
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
  QApplication::setStyle(QStyleFactory::create("Fusion"));

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

  QColor light_gray{160, 160, 160};
  QColor dark_gray{90, 90, 90};
  QColor darker_gray{80, 80, 80};
  darkPalette.setColor(QPalette::Disabled, QPalette::Window, dark_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, light_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::Base, darker_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::AlternateBase, dark_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::ToolTipBase, dark_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::ToolTipText, light_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::Text, light_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::Button, darker_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, light_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::BrightText, light_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::Link, light_gray);
  darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, dark_gray);

  QApplication::setPalette(darkPalette);
  app.setStyleSheet(
      "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px "
      "solid white; }");
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
  // Will be filled by QApplication once instantiated.
  QString path_to_executable;

  // argv might be changed, so we make a copy here!
  auto original_argv = new char*[argc + 1];
  for (int i = 0; i < argc; ++i) {
    const auto size = std::strlen(argv[i]);
    auto dest = new char[size];
    std::strncpy(dest, argv[i], size);
    original_argv[i] = dest;
  }
  original_argv[argc] = nullptr;

  {
    const std::string log_file_path = Path::GetLogFilePath();
    InitLogFile(log_file_path);

    absl::SetProgramUsageMessage("CPU Profiler");
    absl::SetFlagsUsageConfig(
        absl::FlagsUsageConfig{{}, {}, {}, &OrbitCore::GetBuildReport, {}});
    absl::ParseCommandLine(argc, argv);
#if __linux__
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Orbit Profiler [BETA]");
    QCoreApplication::setApplicationVersion(
        QString::fromStdString(OrbitCore::GetVersion()));
    path_to_executable = QCoreApplication::applicationFilePath();

#ifdef ORBIT_CRASH_HANDLING
    const std::string dump_path = Path::GetDumpPath();
#ifdef _WIN32
    const char* handler_name = "crashpad_handler.exe";
#else
    const char* handler_name = "crashpad_handler";
#endif
    const std::string handler_path =
        QDir(QCoreApplication::applicationDirPath())
            .absoluteFilePath(handler_name)
            .toStdString();
    const std::string crash_server_url = CrashServerOptions::GetUrl();
    const std::vector<std::string> attachments = {Path::GetLogFilePath()};

    CrashHandler crash_handler(dump_path, handler_path, crash_server_url,
                               attachments);
#endif  // ORBIT_CRASH_HANDLING

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
              "trying to start Orbit in a remote session and make sure you "
              "have a recent graphics driver installed. Then try again!")
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
      const auto result =
          RunUiInstance(&app, deployment_configuration, &(context.value()));
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
        break;
      }
    }
  }

  execv(path_to_executable.toLocal8Bit().constData(), original_argv);
  UNREACHABLE();
}
