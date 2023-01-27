// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "OrbitBase/Typedef.h"
#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "SftpTestFixture.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_qt_test_utils::YieldsResult;
using orbit_test_utils::HasValue;

using SftpCopyToRemoteOperationTest = SftpTestFixture;

TEST_F(SftpCopyToRemoteOperationTest, Upload) {
  SftpCopyToRemoteOperation upload_operation{GetSession(), GetSftpChannel()};

  const std::filesystem::path upload_source = orbit_test::GetTestdataDir() / "plain.txt";
  const std::filesystem::path upload_destination = "/home/loginuser/upload.txt";
  upload_operation.CopyFileToRemote(upload_source, upload_destination,
                                    SftpCopyToRemoteOperation::FileMode::kUserWritable);

  if (!upload_operation.IsStopped()) {
    QSignalSpy stopped_signal(&upload_operation, &SftpCopyToRemoteOperation::stopped);
    stopped_signal.wait();
  }

  Task task{GetSession(), "cmp /home/loginuser/plain.txt /home/loginuser/upload.txt"};
  EXPECT_THAT(WaitForWithTimeout(task.Execute()), YieldsResult(HasValue(Task::ExitCode{0})));
}
}  // namespace orbit_ssh_qt