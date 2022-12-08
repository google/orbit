// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <numeric>

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "OrbitSshQt/SftpCopyToLocalOperation.h"
#include "SftpTestFixture.h"
#include "Test/Path.h"
#include "TestUtils/TemporaryDirectory.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {

using SftpCopyToLocalOperationTest = SftpTestFixture;

TEST_F(SftpCopyToLocalOperationTest, Download) {
  orbit_base::StopSource stop_source{};

  SftpCopyToLocalOperation operation{GetSession(), GetSftpChannel(), stop_source.GetStopToken()};

  ErrorMessageOr<orbit_test_utils::TemporaryDirectory> temp_dir =
      orbit_test_utils::TemporaryDirectory::Create();
  ASSERT_THAT(temp_dir, orbit_test_utils::HasNoError());

  const std::filesystem::path dest_path = temp_dir.value().GetDirectoryPath() / "plain.txt";
  operation.CopyFileToLocal("/home/loginuser/plain.txt", dest_path);

  if (!operation.IsStopped()) {
    QSignalSpy stopped_signal(&operation, &SftpCopyToLocalOperation::stopped);
    stopped_signal.wait();
  }

  ErrorMessageOr<std::string> downloaded_contents = orbit_base::ReadFileToString(dest_path);
  ASSERT_THAT(downloaded_contents, orbit_test_utils::HasNoError());

  ErrorMessageOr<std::string> expected_contents =
      orbit_base::ReadFileToString(orbit_test::GetTestdataDir() / "plain.txt");
  ASSERT_THAT(expected_contents, orbit_test_utils::HasNoError());

  EXPECT_EQ(downloaded_contents.value(), expected_contents.value());
}
}  // namespace orbit_ssh_qt