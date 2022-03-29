// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "CaptureFileInfo/ItemModel.h"
#include "QtUtils/AssertNoQtLogWarnings.h"

namespace orbit_capture_file_info {

namespace {

template <typename... Args>
CaptureFileInfo CreateCaptureFileInfoAndSetCaptureDuration(absl::Duration capture_length,
                                                           Args... args) {
  CaptureFileInfo capture_file_info{args...};
  capture_file_info.SetCaptureLength(capture_length);
  return capture_file_info;
}

const CaptureFileInfo capture_file_info1{"/path/to/file1"};
const CaptureFileInfo capture_file_info2 =
    CreateCaptureFileInfoAndSetCaptureDuration(absl::Seconds(10), "/path/to/file2");
const CaptureFileInfo capture_file_info3 = CreateCaptureFileInfoAndSetCaptureDuration(
    absl::Minutes(2), "/path/to/file3", QDateTime::fromMSecsSinceEpoch(100000000));
const std::vector<CaptureFileInfo> capture_file_infos{capture_file_info1, capture_file_info2,
                                                      capture_file_info3};

}  // namespace

TEST(CaptureFileInfoItemModel, EmptyModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  ItemModel model{};

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST(CaptureFileInfoItemModel, FilledModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  ItemModel model{};
  model.SetCaptureFileInfos({capture_file_infos});

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST(CaptureFileInfoItemModel, SetCaptureFileInfos) {
  ItemModel model{};
  EXPECT_EQ(model.rowCount(), 0);

  model.SetCaptureFileInfos(capture_file_infos);
  EXPECT_EQ(model.rowCount(), 3);

  model.SetCaptureFileInfos({capture_file_info1});
  EXPECT_EQ(model.rowCount(), 1);

  model.SetCaptureFileInfos(capture_file_infos);
  EXPECT_EQ(model.rowCount(), 3);
}

}  // namespace orbit_capture_file_info