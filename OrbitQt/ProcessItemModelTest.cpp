// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QAbstractItemModelTester>
#include <QtGlobal>

#include "OrbitBase/Logging.h"
#include "ProcessItemModel.h"
#include "gtest/gtest.h"
#include "process.pb.h"

namespace {

class AssertNoQtLogWarnings {
  static void MessageHandlerTest(QtMsgType type, const QMessageLogContext& context,
                                 const QString& msg) {
    static bool NO_WARNING_MSG = true;
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type) {
      case QtDebugMsg:
        LOG("Qt debug message: %s (%s:%u, %s)", localMsg.constData(), file, context.line, function);
        break;
      case QtInfoMsg:
        LOG("Qt info message: %s (%s:%u, %s)", localMsg.constData(), file, context.line, function);
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
  AssertNoQtLogWarnings() { qInstallMessageHandler(MessageHandlerTest); }

  ~AssertNoQtLogWarnings() {
    // Install default message handler
    qInstallMessageHandler(nullptr);
  }
};

}  // namespace

namespace OrbitQt {

TEST(ProcessItemModel, ProcessItemModel) {
  // This installs a QtMessageHandler for this scope. Any warning, critical or fatal message
  // produced by Qt in this scope will produce a GTest fail assertion. (Debug and info messages are
  // printed, but do not lead to a fail). In this scope QAbstractItemModelTester is used to
  // automatically test the ProcessItemModel. This QAbstractModelTester produces these messages and
  // AssertNoQtLogWarnings is necessary to bridge a Qt message to a GTest failure.
  AssertNoQtLogWarnings log_qt_test;

  ProcessItemModel model;
  QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);

  EXPECT_FALSE(model.HasProcesses());

  model.SetProcesses({});
  EXPECT_FALSE(model.HasProcesses());

  orbit_grpc_protos::ProcessInfo process_info_1;
  process_info_1.set_pid(15);
  model.SetProcesses({process_info_1});
  EXPECT_TRUE(model.HasProcesses());
  EXPECT_EQ(model.rowCount(), 1);

  orbit_grpc_protos::ProcessInfo process_info_2;
  process_info_2.set_pid(30);
  model.SetProcesses({process_info_1, process_info_2});
  EXPECT_TRUE(model.HasProcesses());
  EXPECT_EQ(model.rowCount(), 2);

  model.SetProcesses({process_info_2});
  EXPECT_TRUE(model.HasProcesses());
  EXPECT_EQ(model.rowCount(), 1);

  model.Clear();
  EXPECT_FALSE(model.HasProcesses());
  EXPECT_EQ(model.rowCount(), 0);
}

}  // namespace OrbitQt