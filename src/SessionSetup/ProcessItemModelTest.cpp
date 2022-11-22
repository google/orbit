// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <string>

#include "GrpcProtos/process.pb.h"
#include "QtUtils/AssertNoQtLogWarnings.h"
#include "SessionSetup/ProcessItemModel.h"

namespace orbit_session_setup {

TEST(ProcessItemModel, ProcessItemModel) {
  // This installs a QtMessageHandler for this scope. Any warning, critical or fatal message
  // produced by Qt in this scope will produce a GTest fail assertion. (Debug and info messages are
  // printed, but do not lead to a fail). In this scope QAbstractItemModelTester is used to
  // automatically test the ProcessItemModel. This QAbstractModelTester produces these messages and
  // AssertNoQtLogWarnings is necessary to bridge a Qt message to a GTest failure.
  orbit_qt_utils::AssertNoQtLogWarnings log_qt_test;

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

}  // namespace orbit_session_setup