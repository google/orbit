// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QDateTime>
#include <chrono>
#include <filesystem>
#include <thread>

#include "CaptureFileInfo/Manager.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/TestUtils.h"

namespace orbit_capture_file_info {

using orbit_base::HasError;

constexpr const char* kOrgName = "The Orbit Authors";

TEST(CaptureFileInfoManager, Clear) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.Clear");

  Manager manager;

  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  manager.AddOrTouchCaptureFile("test/path1");
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  manager.AddOrTouchCaptureFile("test/path1");
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());
  manager.AddOrTouchCaptureFile("test/path2");
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());
}

TEST(CaptureFileInfoManager, AddOrTouchCaptureFile) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.AddOrTouchCaptureFile");

  Manager manager;
  manager.Clear();
  ASSERT_TRUE(manager.GetCaptureFileInfos().empty());

  // Add 1st file
  const std::filesystem::path path1 = "path/to/file1";
  manager.AddOrTouchCaptureFile(path1);
  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);

  const CaptureFileInfo& capture_file_info_1 = manager.GetCaptureFileInfos()[0];
  EXPECT_EQ(capture_file_info_1.FilePath(), QString::fromStdString(path1.string()));
  // last used was before (or at the same time) than now_time_stamp.
  const QDateTime now_time_stamp = QDateTime::currentDateTime();
  EXPECT_LE(capture_file_info_1.LastUsed(), now_time_stamp);

  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  // Touch 1st file
  manager.AddOrTouchCaptureFile(path1);
  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);

  // last used was after now_time_stamp
  EXPECT_GT(capture_file_info_1.LastUsed(), now_time_stamp);

  // Add 2nd file
  const std::filesystem::path path2 = "path/to/file2";
  manager.AddOrTouchCaptureFile(path2);
  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 2);
  const CaptureFileInfo& capture_file_info_2 = manager.GetCaptureFileInfos()[1];
  EXPECT_EQ(capture_file_info_2.FilePath(), QString::fromStdString(path2.string()));

  // clean up
  manager.Clear();
}

TEST(CaptureFileInfoManager, PurgeNonExistingFiles) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.PurgeNonExistingFiles");

  Manager manager;
  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  manager.AddOrTouchCaptureFile("non/existing/path");
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  manager.PurgeNonExistingFiles();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  const std::filesystem::path existing_file =
      orbit_base::GetExecutableDir() / "testdata" / "CaptureFileInfo" / "test_file.txt";
  manager.AddOrTouchCaptureFile(existing_file);
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  manager.PurgeNonExistingFiles();
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  // clean up
  manager.Clear();
}

TEST(CaptureFileInfoManager, Persistency) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.Persistency");

  {  // clean setup
    Manager manager;
    manager.Clear();
  }

  {
    Manager manager;
    EXPECT_TRUE(manager.GetCaptureFileInfos().empty());
  }

  const std::filesystem::path existing_file =
      orbit_base::GetExecutableDir() / "testdata" / "CaptureFileInfo" / "test_file.txt";
  {
    Manager manager;
    manager.AddOrTouchCaptureFile(existing_file);
    EXPECT_EQ(manager.GetCaptureFileInfos().size(), 1);
  }

  {
    Manager manager;
    ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);
    const CaptureFileInfo capture_file_info = manager.GetCaptureFileInfos()[0];

    std::filesystem::path saved_path = capture_file_info.FilePath().toStdString();

    EXPECT_EQ(saved_path, existing_file);
  }

  {  // clean up
    Manager manager;
    manager.Clear();
  }
}

TEST(CaptureFileInfoManager, FillFromDirectory) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.FillFromDirectory");

  Manager manager;
  manager.Clear();

  {  // Fail
    ErrorMessageOr<void> result = manager.FillFromDirectory("/non/existent/path/to/dir");
    EXPECT_THAT(result, HasError("Unable to list files in directory"));
  }

  {  // Success
    std::filesystem::path test_data_dir =
        orbit_base::GetExecutableDir() / "testdata" / "CaptureFileInfo";
    ErrorMessageOr<void> result = manager.FillFromDirectory(test_data_dir);
    ASSERT_FALSE(result.has_error());

    ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);

    const CaptureFileInfo capture_file_info = manager.GetCaptureFileInfos()[0];

    EXPECT_EQ(capture_file_info.FileName(), "test_capture.orbit");
  }

  // clean up
  manager.Clear();
}

}  // namespace orbit_capture_file_info