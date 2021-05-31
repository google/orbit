// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "CaptureFileInfo/ItemModel.h"
#include "QtUtils/AssertNoQtLogWarnings.h"

namespace orbit_capture_file_info {

TEST(CaptureFileInfoItemModel, EmptyModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  ItemModel model{};

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST(CaptureFileInfoItemModel, FilledModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  ItemModel model{};

  model.SetCaptureFileInfos(
      {CaptureFileInfo{"/path/to/file1"}, CaptureFileInfo{"/path/to/file2"},
       CaptureFileInfo{"/path/to/file3", QDateTime::fromMSecsSinceEpoch(100000000)}});

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

}  // namespace orbit_capture_file_info