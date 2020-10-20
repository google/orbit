
// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QWebChannel>
#include <QWebEngineView>
#include <QtWebEngine>
#include <cstdio>

#include "CodeView.h"
#include "gtest/gtest.h"

static int argc = 0;

TEST(CodeView, LoadPage) {
  std::puts("If you see some message about failed OpenGL context creation, you can ignore those.");
  std::puts(
      "This test does not require OpenGL. If it fails it will probably be because of something "
      "else.");
  QApplication app(argc, nullptr);

  QWebEnginePage page{};

  orbit_qt::CodeView code_view{{}, nullptr};
  code_view.SetTestModeEnabled(true);
  page.setWebChannel(code_view.GetWebEngineView()->GetWebChannel());

  QObject::connect(&page, &QWebEnginePage::printRequested, &app, [&]() {
    std::puts("Received print request - which means everything went fine.");
    SUCCEED();
    app.quit();
  });

  QTimer timeout{};
  QObject::connect(&timeout, &QTimer::timeout, [&]() {
    FAIL() << "Timeout occured: This usually means something went wrong on the JavaScript side.";
    app.exit(1);
  });

  constexpr int timeout_in_msecs = 5000;
  timeout.start(timeout_in_msecs);

  const auto kCodeViewUrl = QStringLiteral("qrc:///WebUI/CodeView/index.html");
  page.load(QUrl{kCodeViewUrl});

  app.exec();
}
