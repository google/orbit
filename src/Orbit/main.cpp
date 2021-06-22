// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/flags/usage_config.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <Qt>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "MetricsUploader/ScopedMetric.h"
#include "MoveFilesToDocuments/MoveFilesToDocuments.h"
#include "OrbitBase/File.h"

#ifdef _WIN32
#include <process.h>
#else
#endif

#include "AccessibilityAdapter.h"
#include "ImGuiOrbit.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/CrashHandler.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitSsh/Context.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitVersion/OrbitVersion.h"
#include "Path.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/DeploymentConfigurations.h"
#include "SessionSetup/SessionSetupDialog.h"
#include "SessionSetup/TargetConfiguration.h"
#include "SessionSetup/servicedeploymanager.h"
#include "SourcePathsMapping/MappingManager.h"
#include "Style/Style.h"
#include "opengldetect.h"
#include "orbitmainwindow.h"

#ifdef ORBIT_CRASH_HANDLING
#include "CrashHandler/CrashHandler.h"
#include "CrashHandler/CrashOptions.h"
#endif

ABSL_DECLARE_FLAG(uint16_t, grpc_port);
ABSL_DECLARE_FLAG(bool, local);
ABSL_DECLARE_FLAG(bool, devmode);

// This flag is needed by the E2E tests to ensure a clean state before running.
ABSL_FLAG(bool, clear_source_paths_mappings, false, "Clear all the stored source paths mappings");

using orbit_session_setup::DeploymentConfiguration;
using orbit_session_setup::NoDeployment;
using orbit_ssh_qt::ScopedConnection;
using GrpcPort = orbit_session_setup::ServiceDeployManager::GrpcPort;
using orbit_ssh::Context;

Q_DECLARE_METATYPE(std::error_code);

void RunUiInstance(const DeploymentConfiguration& deployment_configuration,
                   const Context* ssh_context, const QStringList& command_line_flags,
                   const orbit_base::CrashHandler* crash_handler,
                   const std::filesystem::path& capture_file_path) {
  qRegisterMetaType<std::error_code>();

  const GrpcPort grpc_port{/*.grpc_port =*/absl::GetFlag(FLAGS_grpc_port)};

  orbit_session_setup::SshConnectionArtifacts ssh_connection_artifacts{ssh_context, grpc_port,
                                                                       &deployment_configuration};

  std::optional<orbit_session_setup::TargetConfiguration> target_config;

  std::unique_ptr<orbit_metrics_uploader::MetricsUploader> metrics_uploader =
      orbit_metrics_uploader::MetricsUploader::CreateMetricsUploader();
  metrics_uploader->SendLogEvent(
      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_METRICS_UPLOADER_START);

  orbit_metrics_uploader::ScopedMetric metric{
      metrics_uploader.get(), orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_EXIT};

  // If Orbit starts with loading a capture file, we skip SessionSetupDialog and create a
  // FileTarget from capture_file_path. After creating the FileTarget, we reset
  // skip_profiling_target_dialog as false such that if a user ends the previous session, Orbit
  // will return to a SessionSetupDialog.
  bool skip_profiling_target_dialog = !capture_file_path.empty();
  while (true) {
    {
      if (skip_profiling_target_dialog) {
        target_config = orbit_session_setup::FileTarget(capture_file_path);
        skip_profiling_target_dialog = false;
      } else {
        orbit_session_setup::SessionSetupDialog target_dialog{
            &ssh_connection_artifacts, std::move(target_config), metrics_uploader.get()};
        target_config = target_dialog.Exec();

        if (!target_config.has_value()) {
          // User closed dialog
          break;
        }
      }
    }

    int application_return_code = 0;
    orbit_qt::InstallAccessibilityFactories();

    {  // Scoping of QT UI Resources

      OrbitMainWindow w(std::move(target_config.value()), crash_handler, metrics_uploader.get(),
                        command_line_flags);

      // "resize" is required to make "showMaximized" work properly.
      w.resize(1280, 720);
      w.showMaximized();

      application_return_code = QApplication::exec();

      target_config = w.ClearTargetConfiguration();
    }

    if (application_return_code == OrbitMainWindow::kQuitOrbitReturnCode) {
      // User closed window
      break;
    }

    if (application_return_code == OrbitMainWindow::kEndSessionReturnCode) {
      // User clicked end session, or socket error occurred.
      continue;
    }

    UNREACHABLE();
  }
}

// Extract command line flags by filtering the positional arguments out from the command line
// arguments.
static QStringList ExtractCommandLineFlags(const std::vector<std::string>& command_line_args,
                                           const std::vector<char*>& positional_args) {
  QStringList command_line_flags;
  absl::flat_hash_set<std::string> positional_arg_set(positional_args.begin(),
                                                      positional_args.end());
  for (const std::string& command_line_arg : command_line_args) {
    if (!positional_arg_set.contains(command_line_arg)) {
      command_line_flags << QString::fromStdString(command_line_arg);
    }
  }
  return command_line_flags;
}

static void DisplayErrorToUser(const QString& message) {
  QMessageBox::critical(nullptr, QApplication::applicationName(), message);
}

static bool DevModeEnabledViaEnvironmentVariable() {
  const auto env = QProcessEnvironment::systemEnvironment();
  return env.contains("ORBIT_DEV_MODE") || env.contains("ORBIT_DEVELOPER_MODE");
}

static void LogAndMaybeWarnAboutClockResolution() {
  uint64_t estimated_clock_resolution = orbit_base::EstimateClockResolution();
  LOG("%s", absl::StrFormat("Clock resolution on client: %d (ns)", estimated_clock_resolution));

  // Since a low clock resolution on the client only affects our own introspection and logging
  // timings, we only show a warning dialogue when running in devmode.
  constexpr uint64_t kWarnThresholdClockResolutionNs = 10 * 1000;  // 10 us
  if (absl::GetFlag(FLAGS_devmode) &&
      estimated_clock_resolution > kWarnThresholdClockResolutionNs) {
    DisplayErrorToUser(
        QString("Warning, clock resolution is low (estimated as %1 ns)! Introspection "
                "timings may be inaccurate.")
            .arg(estimated_clock_resolution));
  }
  // An estimated clock resolution of 0 means that estimating the resolution failed. This can
  // happen for really low resolutions and is likely an error case that we should warn about
  // in devmode.
  if (absl::GetFlag(FLAGS_devmode) && estimated_clock_resolution == 0) {
    DisplayErrorToUser(QString(
        "Warning, failed to estimate clock resolution! Introspection timings may be inaccurate."));
  }
}

// Removes all source paths mappings from the persistent settings storage.
static void ClearSourcePathsMappings() {
  orbit_source_paths_mapping::MappingManager mapping_manager{};
  mapping_manager.SetMappings({});
  LOG("Cleared the saved source paths mappings.");
}

// Put the command line into the log.
static void LogCommandLine(int argc, char* argv[]) {
  LOG("Command line invoking Orbit:");
  LOG("%s", argv[0]);
  for (int i = 1; i < argc; i++) {
    LOG("  %s", argv[i]);
  }
  LOG("");
}

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("CPU Profiler");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &orbit_core::GetBuildReport, {}});
  std::vector<char*> positional_args = absl::ParseCommandLine(argc, argv);

  QString orbit_executable = QString(argv[0]);
  std::vector<std::string> command_line_args;
  if (argc > 1) {
    command_line_args.assign(argv + 1, argv + argc);
  }
  QStringList command_line_flags = ExtractCommandLineFlags(command_line_args, positional_args);
  // Skip program name in positional_args[0].
  std::vector<std::string> capture_file_paths(positional_args.begin() + 1, positional_args.end());

  std::filesystem::path log_file{orbit_core::GetLogFilePath()};
  orbit_base::InitLogFile(log_file);
  LOG("You are running Orbit Profiler version %s", orbit_core::GetVersion());
  LogCommandLine(argc, argv);
  ErrorMessageOr<void> remove_old_log_result =
      orbit_base::TryRemoveOldLogFiles(orbit_core::CreateOrGetLogDir());
  if (remove_old_log_result.has_error()) {
    LOG("Warning: Unable to remove some old log files:\n%s",
        remove_old_log_result.error().message());
  }

#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  QApplication app(argc, argv);
  QApplication::setOrganizationName("The Orbit Authors");
  QApplication::setApplicationName("orbitprofiler");

  if (DevModeEnabledViaEnvironmentVariable()) {
    absl::SetFlag(&FLAGS_devmode, true);
  }

  // The application display name is automatically appended to all window titles when shown in the
  // title bar: <specific window title> - <application display name>
  const auto version_string = QString::fromStdString(orbit_core::GetVersion());
  auto display_name = QString{"Orbit Profiler %1 [BETA]"}.arg(version_string);

  if (absl::GetFlag(FLAGS_devmode)) {
    display_name.append(" [DEVELOPER MODE]");
  }

  QApplication::setApplicationDisplayName(display_name);
  QApplication::setApplicationVersion(version_string);

  auto crash_handler = std::make_unique<orbit_base::CrashHandler>();
#ifdef ORBIT_CRASH_HANDLING
  const std::string dump_path = orbit_core::CreateOrGetDumpDir().string();
#ifdef _WIN32
  const char* handler_name = "crashpad_handler.exe";
#else
  const char* handler_name = "crashpad_handler";
#endif
  const std::string handler_path =
      QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(handler_name).toStdString();
  const std::string crash_server_url = orbit_crash_handler::GetServerUrl();
  const std::vector<std::string> attachments = {log_file.string()};

  crash_handler = std::make_unique<orbit_crash_handler::CrashHandler>(
      dump_path, handler_path, crash_server_url, attachments);
#endif  // ORBIT_CRASH_HANDLING

  if (absl::GetFlag(FLAGS_clear_source_paths_mappings)) {
    ClearSourcePathsMappings();
    return 0;
  }

  orbit_style::ApplyStyle(&app);

  const auto open_gl_version = orbit_qt::DetectOpenGlVersion();

  if (!open_gl_version) {
    DisplayErrorToUser(
        "OpenGL support was not found. This usually indicates some DLLs are missing. "
        "Please try to reinstall Orbit!");
    return -1;
  }

  LOG("Detected OpenGL version: %i.%i %s", open_gl_version->major, open_gl_version->minor,
      open_gl_version->is_opengl_es ? "OpenGL ES" : "OpenGL");

  if (open_gl_version->is_opengl_es) {
    DisplayErrorToUser(
        "Orbit was only able to load OpenGL ES while Desktop OpenGL is required. Try to force "
        "software rendering by starting Orbit with the environment variable QT_OPENGL=software "
        "set.");
    return -1;
  }

  if (open_gl_version->major < 2) {
    DisplayErrorToUser(QString("The minimum required version of OpenGL is 2.0. But this machine "
                               "only supports up to version %1.%2. Please make sure you're not "
                               "trying to start Orbit in a remote session and make sure you "
                               "have a recent graphics driver installed. Then try again!")
                           .arg(open_gl_version->major)
                           .arg(open_gl_version->minor));
    return -1;
  }

  LogAndMaybeWarnAboutClockResolution();

  orbit_move_files_to_documents::TryMoveSavedDataLocationIfNeeded();

  const DeploymentConfiguration deployment_configuration =
      orbit_session_setup::FigureOutDeploymentConfiguration();

  auto context = Context::Create();
  if (!context) {
    DisplayErrorToUser(QString("An error occurred while initializing ssh: %1")
                           .arg(QString::fromStdString(context.error().message())));
    return -1;
  }

  // If more than one capture files are provided, start multiple Orbit instances.
  for (size_t i = 1; i < capture_file_paths.size(); ++i) {
    QStringList arguments;
    arguments << QString::fromStdString(capture_file_paths[i]) << command_line_flags;
    QProcess::startDetached(orbit_executable, arguments);
  }

  RunUiInstance(deployment_configuration, &context.value(), command_line_flags, crash_handler.get(),
                capture_file_paths.empty() ? "" : capture_file_paths[0]);
  return 0;
}
