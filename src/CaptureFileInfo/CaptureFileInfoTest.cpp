// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QDateTime>
#include <string_view>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "OrbitBase/ExecutablePath.h"

namespace orbit_capture_file_info {

TEST(CaptureFileInfo, PathConstructor) {
  const QString kParentPath{"this/is/a/test/path/"};
  const QString kFileName{"example file name.extension"};
  const QString kFullPath{kParentPath + kFileName};

  CaptureFileInfo capture_file_info{kFullPath};

  EXPECT_EQ(capture_file_info.FilePath(), kFullPath);
  EXPECT_EQ(capture_file_info.FileName(), kFileName);

  // LastUsed() before or equal to now.
  EXPECT_LE(capture_file_info.LastUsed(), QDateTime::currentDateTime());
}

TEST(CaptureFileInfo, PathLastUsedConstructor) {
  const QString kParentPath{"this/is/a/test/path/"};
  const QString kFileName{"example file name.extension"};
  const QString kFullPath{kParentPath + kFileName};
  const QDateTime last_used = QDateTime::fromMSecsSinceEpoch(1600000000000);

  CaptureFileInfo capture_file_info{kFullPath, last_used};

  EXPECT_EQ(capture_file_info.FilePath(), kFullPath);
  EXPECT_EQ(capture_file_info.FileName(), kFileName);

  EXPECT_EQ(capture_file_info.LastUsed(), last_used);
}

TEST(CaptureFileInfo, FileExistsAndCreated) {
  {
    const std::filesystem::path path =
        orbit_base::GetExecutableDir() / "testdata" / "test_file.txt";

    CaptureFileInfo capture_file_info{QString::fromStdString(path.string())};

    ASSERT_TRUE(capture_file_info.FileExists());

    // File was created before (or equal to) now.
    EXPECT_LE(capture_file_info.Created(), QDateTime::currentDateTime());
  }

  {
    const std::filesystem::path path =
        orbit_base::GetExecutableDir() / "testdata" / "not_existing_test_file.txt";

    CaptureFileInfo capture_file_info{QString::fromStdString(path.string())};

    ASSERT_FALSE(capture_file_info.FileExists());

    QDateTime invalid_time;
    EXPECT_EQ(capture_file_info.Created(), invalid_time);
  }
}

TEST(CaptureFileInfo, Touch) {
  const QString path{"test/path/file.ext"};
  const QDateTime last_used = QDateTime::fromMSecsSinceEpoch(1600000000000);

  CaptureFileInfo capture_file_info{path, last_used};

  EXPECT_EQ(capture_file_info.LastUsed(), last_used);

  QDateTime now = QDateTime::currentDateTime();

  // last used was before now
  EXPECT_LT(capture_file_info.LastUsed(), now);

  capture_file_info.Touch();

  // last used after or equal to now
  EXPECT_GE(capture_file_info.LastUsed(), now);
}

}  // namespace orbit_capture_file_info