//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QStyleFactory>

#include "App.h"
#include "ApplicationOptions.h"
#include "CrashHandler.h"
#include "OrbitStartupWindow.h"
#include "Path.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "orbitmainwindow.h"

// TODO: Remove this flag once we have a dialog with user
ABSL_FLAG(bool, upload_dumps_to_server, false,
          "Upload dumps to collection server when crashes");

ABSL_FLAG(std::string, remote, "",
          "Connect to the specified remote on startup");

constexpr const uint16_t kDefaultAsioPort = 44766;

// TODO: remove this once we deprecated legacy parameters
static void ParseLegacyCommandLine(int argc, char* argv[],
                                   ApplicationOptions* options) {
  for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
    const char* arg = argv[i];
    if (absl::StartsWith(arg, "gamelet:")) {
      std::cerr << "WARNING: the 'gamelet:<host>:<port>' option is deprecated, "
                   "please use --gamelet <host> instead."
                << std::endl;
      options->asio_server_address = arg + std::strlen("gamelet:");
    }
  }
}

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("CPU Profiler");
  absl::ParseCommandLine(argc, argv);
#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  QApplication app(argc, argv);

  const std::string dump_path = Path::GetDumpPath();
#ifdef _WIN32
  const char* handler_name = "crashpad_handler.exe";
#else
  const char* handler_name = "crashpad_handler";
#endif
  const std::string handler_path = QDir(QCoreApplication::applicationDirPath())
                                       .absoluteFilePath(handler_name)
                                       .toStdString();
  const std::string crash_server_url =
      "https://clients2.google.com/cr/staging_report";
  const CrashHandler crash_handler(dump_path, handler_path, crash_server_url,
                                   absl::GetFlag(FLAGS_upload_dumps_to_server));

  ApplicationOptions options;

  ParseLegacyCommandLine(argc, argv, &options);
  std::string remote = absl::GetFlag(FLAGS_remote);
  if (!remote.empty()) {
    // Append default port only if the user has not specified one
    if (!absl::StrContains(remote, ":")) {
      remote = absl::StrFormat("%s:%d", remote, kDefaultAsioPort);
    }

    options.asio_server_address = remote;
  }

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

  if (options.asio_server_address.empty()) {
    OrbitStartupWindow sw;
    std::string ip_address;
    int dialog_result = sw.Run(&ip_address);
    if (dialog_result == 0) return 0;

#ifdef __linux__
    options.asio_server_address =
        absl::StrFormat("%s:%d", ip_address, kDefaultAsioPort);
#else
    // TODO(antonrohr) remove this ifdef as soon as the collector works on
    // windows
    if (ip_address != "127.0.0.1") {
      options.asio_server_address =
          absl::StrFormat("%s:%d", ip_address, kDefaultAsioPort);
    }
#endif
  }

  OrbitMainWindow w(&app, std::move(options));

  if (!w.IsHeadless()) {
    w.showMaximized();
  } else {
    w.show();
    w.hide();
  }

  w.PostInit();

  int errorCode = app.exec();

  OrbitApp::OnExit();

  return errorCode;
}
