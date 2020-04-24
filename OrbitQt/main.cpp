//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QStyleFactory>

#include "../OrbitGl/App.h"
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

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("CPU Profiler");
  absl::ParseCommandLine(argc, argv);
#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  QApplication a(argc, argv);

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

  a.setStyle(QStyleFactory::create("Fusion"));

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
  a.setPalette(darkPalette);
  a.setStyleSheet(
      "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid "
      "white; }");

  // TODO(antonrohr) refactor argument parsing; probably use a external argument
  // parsing library
  std::vector<std::string> arguments;
  bool found_gamelet_arg = false;
  for (const QString& arg : QApplication::arguments()) {
    if (arg.contains("gamelet:")) found_gamelet_arg = true;
    arguments.emplace_back(arg.toStdString());
  }

  if (!found_gamelet_arg) {
    OrbitStartupWindow sw;
    std::string ip_address;
    int dialog_result = sw.Run(&ip_address);
    if (dialog_result == 0) return 0;

#ifdef __linux__
    arguments.emplace_back("gamelet:" + ip_address + ":44766");
#else
    // TODO(antonrohr) remove this ifdef as soon as the collector works on
    // windows
    if (ip_address != "127.0.0.1") {
      arguments.emplace_back("gamelet:" + ip_address + ":44766");
    }
#endif
  }

  OrbitMainWindow w(arguments, &a);

  if (!w.IsHeadless()) {
    w.showMaximized();
  } else {
    w.show();
    w.hide();
  }

  w.PostInit();

  int errorCode = a.exec();

  OrbitApp::OnExit();

  return errorCode;
}
