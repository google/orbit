// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QList>
#include <QSignalSpy>
#include <QVariant>
#include <filesystem>
#include <string>

#include "OrbitSshQt/SftpCopyToRemoteOperation.h"
#include "OrbitSshQt/Task.h"
#include "QtTestUtils/WaitFor.h"
#include "SftpTestFixture.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {
using orbit_qt_test_utils::WaitFor;
using orbit_qt_test_utils::YieldsResult;
using orbit_test_utils::HasNoError;

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
  QSignalSpy finished_signal{&task, &orbit_ssh_qt::Task::finished};
  EXPECT_THAT(WaitFor(task.Start()), YieldsResult(HasNoError()));
  EXPECT_THAT(WaitFor(task.Stop()), YieldsResult(HasNoError()));

  ASSERT_EQ(finished_signal.size(), 1);
  // Return value 0 means there was no difference in the two compared files. Hence the upload
  // succeeded.
  EXPECT_EQ(finished_signal[0][0].toInt(), 0);
}
}  // namespace orbit_ssh_qt