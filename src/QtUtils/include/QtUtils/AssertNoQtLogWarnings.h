// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_ASSERT_NO_QT_LOG_WARNINGS_H_
#define QT_UTILS_ASSERT_NO_QT_LOG_WARNINGS_H_

#include <gtest/gtest.h>

#include <QByteArray>
#include <QMessageLogContext>
#include <QString>
#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_qt_utils {

// This class installs a QtMessageHandler as long as it is alive. Any warning, critical or fatal
// message produced by Qt in this scope will produce a GTest failed assertion. (Debug and info
// messages are printed, but do not lead to a fail).
//
// This can be used to map QAbstractItemModelTester warning messages to gtest failed assertions.
class AssertNoQtLogWarnings {
  static void MessageHandlerTest(QtMsgType type, const QMessageLogContext& context,
                                 const QString& msg) {
    static bool NO_WARNING_MSG = true;
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file != nullptr ? context.file : "";
    const char* function = context.function != nullptr ? context.function : "";
    switch (type) {
      case QtDebugMsg:
        ORBIT_LOG("Qt debug message: %s (%s:%u, %s)", localMsg.constData(), file, context.line,
                  function);
        break;
      case QtInfoMsg:
        ORBIT_LOG("Qt info message: %s (%s:%u, %s)", localMsg.constData(), file, context.line,
                  function);
        break;
      case QtWarningMsg:
        EXPECT_EQ(false, NO_WARNING_MSG) << msg.toStdString();
        break;
      case QtCriticalMsg:
        EXPECT_EQ(false, NO_WARNING_MSG) << msg.toStdString();
        break;
      case QtFatalMsg:
        EXPECT_EQ(false, NO_WARNING_MSG) << msg.toStdString();
        break;
    }
  }

 public:
  explicit AssertNoQtLogWarnings() { qInstallMessageHandler(MessageHandlerTest); }

  ~AssertNoQtLogWarnings() {
    // Install default message handler
    qInstallMessageHandler(nullptr);
  }
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_ASSERT_NO_QT_LOG_WARNINGS_H_