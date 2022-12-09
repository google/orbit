// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QModelIndex>
#include <QPoint>
#include <QTableView>
#include <QTest>
#include <QVariant>
#include <Qt>
#include <memory>

#include "ClientData/CaptureData.h"
#include "OrbitGl/StaticTimeGraphLayout.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/TrackManager.h"
#include "OrbitGl/TrackTestData.h"
#include "OrbitQt/TrackTypeItemModel.h"
#include "QtUtils/AssertNoQtLogWarnings.h"

namespace orbit_qt {

class TrackTypeItemModelTest : public ::testing::Test {
 protected:
  int FindRowByTrackType(Track::Type track_type) {
    int found_row = -1;

    for (int i = 0; i < model_.rowCount(); ++i) {
      auto this_row_type =
          model_.data(model_.index(i, 0), TrackTypeItemModel::kTrackTypeRole).value<Track::Type>();
      if (track_type == this_row_type) {
        found_row = i;
      }
    }

    return found_row;
  }

  // This installs a QtMessageHandler for this scope. Any warning, critical or fatal message
  // produced by Qt in this scope will produce a GTest fail assertion. (Debug and info messages are
  // printed, but do not lead to a fail). In this scope QAbstractItemModelTester is used to
  // automatically test the ProcessItemModel. This QAbstractModelTester produces these messages and
  // AssertNoQtLogWarnings is necessary to bridge a Qt message to a GTest failure.
  orbit_qt_utils::AssertNoQtLogWarnings log_qt_test_;
  TrackTypeItemModel model_;

  orbit_gl::StaticTimeGraphLayout layout_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_ =
      orbit_gl::TrackTestData::GenerateTestCaptureData();
  orbit_gl::TrackManager track_manager_ = orbit_gl::TrackManager(
      nullptr, nullptr, nullptr, &layout_, nullptr, nullptr, capture_data_.get());
};

TEST_F(TrackTypeItemModelTest, QtBasicTests) {
  QAbstractItemModelTester tester(&model_, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST_F(TrackTypeItemModelTest, ReadAndWriteData) {
  EXPECT_EQ(0, model_.rowCount());

  // When a track manager is set, the data should be non-empty
  model_.SetTrackManager(&track_manager_);
  EXPECT_GT(model_.rowCount(), 0);

  // Search for the "ThreadTrack" entry and make sure it exists.
  // This is an arbitrary example of a track type that we expect to be in the model.
  int found_row = FindRowByTrackType(Track::Type::kThreadTrack);
  EXPECT_TRUE(found_row >= 0);
  constexpr int kNameCol = static_cast<int>(TrackTypeItemModel::Column::kName);
  constexpr int kVisCol = static_cast<int>(TrackTypeItemModel::Column::kVisibility);
  QModelIndex name_index = model_.index(found_row, kNameCol);
  QModelIndex vis_index = model_.index(found_row, kVisCol);

  // Check the colum contents:
  // - Expect the "name" column to be non-empty
  // - Expect the "visibility" column to be checked depending on the visibility
  EXPECT_NE("", model_.data(name_index, Qt::DisplayRole).toString());
  EXPECT_EQ(Qt::Checked, model_.data(vis_index, Qt::CheckStateRole).value<Qt::CheckState>());

  // When track visibility changes, the check state changes as well
  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, false);
  EXPECT_EQ(Qt::Unchecked, model_.data(vis_index, Qt::CheckStateRole).value<Qt::CheckState>());
}

TEST_F(TrackTypeItemModelTest, ViewInteraction) {
  model_.SetTrackManager(&track_manager_);
  int found_row = FindRowByTrackType(Track::Type::kThreadTrack);
  EXPECT_TRUE(found_row >= 0);

  QTableView table;
  table.setModel(&model_);
  EXPECT_TRUE(track_manager_.GetTrackTypeVisibility(Track::Type::kThreadTrack));
  int x =
      table.columnViewportPosition(static_cast<int>(TrackTypeItemModel::Column::kVisibility)) + 15;
  int y = table.rowViewportPosition(found_row) + 20;
  QTest::mouseClick(table.viewport(), Qt::MouseButton::LeftButton, {0}, QPoint(x, y));
  EXPECT_FALSE(track_manager_.GetTrackTypeVisibility(Track::Type::kThreadTrack));
}

}  // namespace orbit_qt