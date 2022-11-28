// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QString>
#include <chrono>
#include <filesystem>
#include <optional>
#include <set>
#include <thread>
#include <utility>
#include <vector>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "CaptureFileInfo/Manager.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_capture_file_info {

using orbit_test_utils::HasError;

constexpr const char* kOrgName = "The Orbit Authors";

TEST(CaptureFileInfoManager, Clear) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.Clear");

  Manager manager;

  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  manager.AddOrTouchCaptureFile("test/path1", std::nullopt);
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  manager.AddOrTouchCaptureFile("test/path1", std::nullopt);
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());
  manager.AddOrTouchCaptureFile("test/path2", std::nullopt);
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
  manager.AddOrTouchCaptureFile(path1, std::nullopt);
  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);

  const CaptureFileInfo& capture_file_info_1 = manager.GetCaptureFileInfos()[0];
  EXPECT_EQ(capture_file_info_1.FilePath(), QString::fromStdString(path1.string()));
  // last used was before (or at the same time) than now_time_stamp.
  const QDateTime now_time_stamp = QDateTime::currentDateTime();
  EXPECT_LE(capture_file_info_1.LastUsed(), now_time_stamp);

  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  // Touch 1st file
  const absl::Duration capture_length1 = absl::Seconds(10);
  manager.AddOrTouchCaptureFile(path1, capture_length1);
  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);
  ASSERT_EQ(manager.GetCaptureFileInfos()[0].CaptureLength(), capture_length1);

  // last used was after now_time_stamp
  EXPECT_GT(capture_file_info_1.LastUsed(), now_time_stamp);

  // Add 2nd file
  const std::filesystem::path path2 = "path/to/file2";
  const absl::Duration capture_length2 = absl::Milliseconds(10);
  manager.AddOrTouchCaptureFile(path2, capture_length2);
  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 2);
  const CaptureFileInfo& capture_file_info_2 = manager.GetCaptureFileInfos()[1];
  EXPECT_EQ(capture_file_info_2.FilePath(), QString::fromStdString(path2.string()));
  EXPECT_EQ(capture_file_info_2.CaptureLength(), capture_length2);

  // clean up
  manager.Clear();
}

TEST(CaptureFileInfoManager, AddOrTouchCaptureFilePathFormatting) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName(
      "CaptureFileInfo.Manager.AddOrTouchCaptureFilePathFormatting");

  Manager manager;
  manager.Clear();
  ASSERT_TRUE(manager.GetCaptureFileInfos().empty());

  // Currently AddOrTouchCaptureFile uses path operator== to discern whether 2 paths are the same.
  // This test uses the same mechanism (via std::set), which makes this more of a smoke test.
  // TODO(http://b/218298681) use std::filesystem::equivalent instead of operator== and improve this
  // test.

  std::vector<std::filesystem::path> test_paths = {"/path/to/some/file",
                                                   "/PATH/TO/SOME/FILE",
                                                   "c:/users/user/dir/file.orbit",
                                                   "C:/users/user/dir/file.orbit",
                                                   "c:/users/user/dir/file.ORBIT",
                                                   "C:/USERS/USER/DIR/FILE.ORBIT",
                                                   R"(c:\users\user\dir\file.orbit)",
                                                   R"(C:\USERS\USER\DIR\FILE.ORBIT)"};

  std::set<std::filesystem::path> control_set;
  for (const auto& path : test_paths) {
    manager.AddOrTouchCaptureFile(path, std::nullopt);
    control_set.insert(path);
  }

  EXPECT_EQ(manager.GetCaptureFileInfos().size(), control_set.size());
}

TEST(CaptureFileInfoManager, PurgeNonExistingFiles) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.PurgeNonExistingFiles");

  Manager manager;
  manager.Clear();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  manager.AddOrTouchCaptureFile("non/existing/path", std::nullopt);
  EXPECT_FALSE(manager.GetCaptureFileInfos().empty());

  manager.PurgeNonExistingFiles();
  EXPECT_TRUE(manager.GetCaptureFileInfos().empty());

  const std::filesystem::path existing_file = orbit_test::GetTestdataDir() / "test_file.txt";
  manager.AddOrTouchCaptureFile(existing_file, std::nullopt);
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

  const std::filesystem::path existing_file = orbit_test::GetTestdataDir() / "test_file.txt";
  {
    Manager manager;
    manager.AddOrTouchCaptureFile(existing_file, std::nullopt);
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
    std::filesystem::path test_data_dir = orbit_test::GetTestdataDir();
    ErrorMessageOr<void> result = manager.FillFromDirectory(test_data_dir);
    ASSERT_FALSE(result.has_error());

    ASSERT_EQ(manager.GetCaptureFileInfos().size(), 1);

    const CaptureFileInfo capture_file_info = manager.GetCaptureFileInfos()[0];

    EXPECT_EQ(capture_file_info.FileName(), "test_capture.orbit");
  }

  // clean up
  manager.Clear();
}

TEST(CaptureFileInfoManager, GetCaptureLengthByPath) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.GetCaptureLengthByPath");

  Manager manager;
  manager.Clear();

  const std::filesystem::path path1 = "path/to/file1";
  manager.AddOrTouchCaptureFile(path1, std::nullopt);
  ASSERT_FALSE(manager.GetCaptureLengthByPath(path1).has_value());

  const std::filesystem::path path2 = "path/to/file2";
  ASSERT_FALSE(manager.GetCaptureLengthByPath(path2).has_value());

  const absl::Duration capture_length = absl::Milliseconds(10);
  manager.AddOrTouchCaptureFile(path2, capture_length);
  ASSERT_EQ(manager.GetCaptureLengthByPath(path2).value(), capture_length);

  // clean up
  manager.Clear();
}

namespace {

class FakeManager : public Manager {
 public:
  void SetCaptureFileInfos(std::vector<CaptureFileInfo> capture_file_infos) {
    capture_file_infos_ = std::move(capture_file_infos);
  }
};

}  // namespace

TEST(CaptureFileInfoManager, ProcessOutOfSyncFiles) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("CaptureFileInfo.Manager.ProcessOutOfSyncFiles");

  FakeManager manager;
  manager.Clear();

  const QString path1{"path/to/file1"};
  const QDateTime last_used1 = QDateTime::fromMSecsSinceEpoch(1600000000000);
  const QDateTime last_modified1 = QDateTime::fromMSecsSinceEpoch(1500000000000);
  const uint64_t file_size1 = 1234;
  const absl::Duration capture_length1 = absl::Seconds(5);
  CaptureFileInfo capture_file_info1{path1, last_used1, last_modified1, file_size1,
                                     capture_length1};
  EXPECT_TRUE(capture_file_info1.IsOutOfSync());

  const QString path2{"path/to/file2"};
  const absl::Duration capture_length2{absl::Seconds(15)};
  CaptureFileInfo capture_file_info2{path2, capture_length2};
  EXPECT_FALSE(capture_file_info2.IsOutOfSync());

  manager.SetCaptureFileInfos({capture_file_info1, capture_file_info2});

  ASSERT_EQ(manager.GetCaptureFileInfos().size(), 2);
  ASSERT_EQ(manager.GetCaptureFileInfos()[0].CaptureLength().value(), capture_length1);
  ASSERT_EQ(manager.GetCaptureFileInfos()[1].CaptureLength().value(), capture_length2);

  manager.ProcessOutOfSyncFiles();
  ASSERT_FALSE(manager.GetCaptureFileInfos()[0].CaptureLength().has_value());
  ASSERT_EQ(manager.GetCaptureFileInfos()[1].CaptureLength().value(), capture_length2);

  // clean up
  manager.Clear();
}

}  // namespace orbit_capture_file_info