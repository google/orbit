//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>

#include "../OrbitGl/App.h"
#include "orbitmainwindow.h"

int main(int argc, char* argv[]) {
#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  QApplication a(argc, argv);

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

  OrbitMainWindow w(&a);

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
