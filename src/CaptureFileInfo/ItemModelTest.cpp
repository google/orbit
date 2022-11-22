// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QDateTime>
#include <optional>
#include <vector>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "CaptureFileInfo/ItemModel.h"
#include "QtUtils/AssertNoQtLogWarnings.h"

namespace orbit_capture_file_info {

namespace {

const CaptureFileInfo capture_file_info1{"/path/to/file1", std::nullopt};
const CaptureFileInfo capture_file_info2{"/path/to/file2", absl::Seconds(10)};
const CaptureFileInfo capture_file_info3{
    "/path/to/file3", QDateTime::fromMSecsSinceEpoch(100000000), absl::Minutes(2)};
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