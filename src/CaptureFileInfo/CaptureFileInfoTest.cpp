// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <QDateTime>
#include <QString>
#include <filesystem>
#include <memory>
#include <optional>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "Test/Path.h"

namespace orbit_capture_file_info {

TEST(CaptureFileInfo, PathConstructor) {
  const QString parent_path{"this/is/a/test/path/"};
  const QString file_name{"example file name.extension"};
  const QString full_path{parent_path + file_name};
  const absl::Duration capture_length{absl::Seconds(10)};

  CaptureFileInfo capture_file_info{full_path, capture_length};

  EXPECT_EQ(capture_file_info.FilePath(), full_path);
  EXPECT_EQ(capture_file_info.FileName(), file_name);
  EXPECT_EQ(capture_file_info.CaptureLength().value(), capture_length);

  // LastUsed() before or equal to now.
  EXPECT_LE(capture_file_info.LastUsed(), QDateTime::currentDateTime());

  // `last_modified_` and `file_size_` are just created from `file_info_`. Hence these file
  // information are up-to-date.
  EXPECT_FALSE(capture_file_info.IsOutOfSync());
}

TEST(CaptureFileInfo, PathLastUsedConstructor) {
  const QString parent_path{"this/is/a/test/path/"};
  const QString file_name{"example file name.extension"};
  const QString full_path{parent_path + file_name};
  const QDateTime last_used = QDateTime::fromMSecsSinceEpoch(1600000000000);
  const absl::Duration capture_length{absl::Seconds(5)};

  CaptureFileInfo capture_file_info{full_path, last_used, capture_length};

  EXPECT_EQ(capture_file_info.FilePath(), full_path);
  EXPECT_EQ(capture_file_info.FileName(), file_name);
  EXPECT_EQ(capture_file_info.CaptureLength().value(), capture_length);

  EXPECT_EQ(capture_file_info.LastUsed(), last_used);

  EXPECT_FALSE(capture_file_info.IsOutOfSync());
}

TEST(CaptureFileInfo, FullInfoConstructorAndIsOutOfSync) {
  const std::filesystem::path test_full_path = orbit_test::GetTestdataDir() / "test_file.orbit";
  const QDateTime last_used = QDateTime::fromMSecsSinceEpoch(1600000000000);
  const QDateTime last_modified = QDateTime::fromMSecsSinceEpoch(1500000000000);
  const uint64_t file_size = 1234;
  const absl::Duration capture_length{absl::Seconds(5)};

  CaptureFileInfo capture_file_info{QString::fromStdString(test_full_path.string()), last_used,
                                    last_modified, file_size, capture_length};

  EXPECT_EQ(capture_file_info.FilePath(), QString::fromStdString(test_full_path.string()));
  EXPECT_EQ(capture_file_info.FileName(),
            QString::fromStdString(test_full_path.filename().string()));
  EXPECT_EQ(capture_file_info.LastUsed(), last_used);
  EXPECT_EQ(capture_file_info.LastModified(), last_modified);
  EXPECT_EQ(capture_file_info.FileSize(), file_size);
  EXPECT_EQ(capture_file_info.CaptureLength().value(), capture_length);

  // The file size and the last modified time we provided to construct the CaptureFileInfo do not
  // match with the information retrieved from the file system. Hence they are out of sync with the
  // associated file.
  EXPECT_TRUE(capture_file_info.IsOutOfSync());

  capture_file_info.Touch();
  EXPECT_FALSE(capture_file_info.IsOutOfSync());
}

TEST(CaptureFileInfo, FileExistsAndCreated) {
  {
    const std::filesystem::path path = orbit_test::GetTestdataDir() / "test_file.txt";

    CaptureFileInfo capture_file_info{QString::fromStdString(path.string()), std::nullopt};

    ASSERT_TRUE(capture_file_info.FileExists());

    // File was created before (or equal to) now.
    EXPECT_LE(capture_file_info.Created(), QDateTime::currentDateTime());
  }

  {
    const std::filesystem::path path = orbit_test::GetTestdataDir() / "not_existing_test_file.txt";

    CaptureFileInfo capture_file_info{QString::fromStdString(path.string()), std::nullopt};

    ASSERT_FALSE(capture_file_info.FileExists());

    QDateTime invalid_time;
    EXPECT_EQ(capture_file_info.Created(), invalid_time);
  }
}

TEST(CaptureFileInfo, Touch) {
  const QString path{"test/path/file.ext"};
  const QDateTime last_used = QDateTime::fromMSecsSinceEpoch(1600000000000);

  CaptureFileInfo capture_file_info{path, last_used, std::nullopt};

  EXPECT_EQ(capture_file_info.LastUsed(), last_used);

  QDateTime now = QDateTime::currentDateTime();

  // last used was before now
  EXPECT_LT(capture_file_info.LastUsed(), now);

  capture_file_info.Touch();

  // last used after or equal to now
  EXPECT_GE(capture_file_info.LastUsed(), now);
}

TEST(CaptureFileInfo, FileSize) {
  {
    const QString non_existing_path{"test/path/file.ext"};

    CaptureFileInfo capture_file_info{non_existing_path, std::nullopt};

    EXPECT_EQ(capture_file_info.FileSize(), 0);
  }

  {
    const std::filesystem::path path = orbit_test::GetTestdataDir() / "test_file.txt";

    CaptureFileInfo capture_file_info{QString::fromStdString(path.string()), std::nullopt};

    // Not testing exact file size here, because it might be slightly different on windows and linux
    EXPECT_GT(capture_file_info.FileSize(), 0);
  }
}

}  // namespace orbit_capture_file_info