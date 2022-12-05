// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QList>
#include <QRect>
#include <QSignalSpy>
#include <QString>
#include <QTableView>
#include <QTest>
#include <QVariant>
#include <Qt>
#include <memory>

#include "GrpcProtos/process.pb.h"
#include "SessionSetup/OverlayWidget.h"
#include "SessionSetup/ProcessListWidget.h"

namespace orbit_session_setup {

using orbit_grpc_protos::ProcessInfo;

class ProcessListWidgetTest : public ::testing::Test {
  void SetUp() override {
    widget_.show();

    table_view_ = widget_.findChild<QTableView*>("tableView");
    ASSERT_NE(table_view_, nullptr);

    filter_line_edit_ = widget_.findChild<QLineEdit*>("filterLineEdit");
    ASSERT_NE(filter_line_edit_, nullptr);

    overlay_ = widget_.findChild<OverlayWidget*>("overlay");
    ASSERT_NE(overlay_, nullptr);

    test_process_info_1_.set_pid(100);
    test_process_info_1_.set_name("name1");
    test_process_info_1_.set_cpu_usage(10.0);
    test_process_info_1_.set_full_path("full/path/name1");
    test_process_info_1_.set_command_line("example cmd line call1");
    test_process_info_1_.set_is_64_bit(true);
    test_process_info_1_.set_build_id("example build id1");

    test_process_info_2_.set_pid(200);
    test_process_info_2_.set_name("name2");
    test_process_info_2_.set_cpu_usage(20.0);
    test_process_info_2_.set_full_path("full/path/name2");
    test_process_info_2_.set_command_line("example cmd line call2");
    test_process_info_2_.set_is_64_bit(true);
    test_process_info_2_.set_build_id("example build id2");
  }

 protected:
  ProcessListWidget widget_;
  QTableView* table_view_ = nullptr;
  QLineEdit* filter_line_edit_ = nullptr;
  OverlayWidget* overlay_ = nullptr;
  ProcessInfo test_process_info_1_;
  ProcessInfo test_process_info_2_;
};

TEST_F(ProcessListWidgetTest, Clear) {
  // default
  EXPECT_EQ(table_view_->model()->rowCount(), 0);
  EXPECT_TRUE(filter_line_edit_->text().isEmpty());
  EXPECT_FALSE(overlay_->isVisible());

  // Clear at this point does not change anything
  widget_.Clear();
  EXPECT_EQ(table_view_->model()->rowCount(), 0);
  EXPECT_TRUE(filter_line_edit_->text().isEmpty());
  EXPECT_FALSE(overlay_->isVisible());

  // Clear does not change the filter_line_edit_ content
  filter_line_edit_->setText("example filter text");
  widget_.Clear();
  EXPECT_EQ(filter_line_edit_->text(), "example filter text");

  // reset filter
  filter_line_edit_->setText({});

  // add test process
  widget_.UpdateList({test_process_info_1_});
  EXPECT_EQ(table_view_->model()->rowCount(), 1);
  // After the first time adding process list, the cpu values are not reliable and therefore
  // overlay_ is shown.
  EXPECT_TRUE(overlay_->isVisible());

  // Clear
  widget_.Clear();
  EXPECT_EQ(table_view_->model()->rowCount(), 0);
  EXPECT_FALSE(overlay_->isVisible());
}

TEST_F(ProcessListWidgetTest, Overlay) {
  // default
  EXPECT_FALSE(overlay_->isVisible());

  // overlay visible after first list update
  widget_.UpdateList({test_process_info_1_});
  EXPECT_TRUE(overlay_->isVisible());

  // overlay not visible anymore after second list update
  widget_.UpdateList({test_process_info_1_});
  EXPECT_FALSE(overlay_->isVisible());
}

TEST_F(ProcessListWidgetTest, AutoSelection) {
  QSignalSpy selected_spy(&widget_, &ProcessListWidget::ProcessSelected);

  widget_.UpdateList({test_process_info_1_, test_process_info_2_});
  // no selection after first update
  EXPECT_TRUE(selected_spy.empty());

  widget_.UpdateList({test_process_info_1_, test_process_info_2_});
  // auto selection of one process has happened.
  ASSERT_EQ(selected_spy.count(), 1);
  QVariant argument = selected_spy.takeFirst().at(0);
  ASSERT_TRUE(argument.canConvert<ProcessInfo>());
  auto process_info = argument.value<ProcessInfo>();
  // test_process_info_2_ has a higher cpu usage, hence it was selected
  EXPECT_TRUE(
      google::protobuf::util::MessageDifferencer::Equals(process_info, test_process_info_2_));

  selected_spy.clear();

  // Updating the list will emit signal again
  widget_.UpdateList({test_process_info_1_, test_process_info_2_});
  EXPECT_EQ(selected_spy.count(), 1);

  selected_spy.clear();

  // When the selected process disappears from the list, the next highest cpu process will be
  // selected. (In the following case process_1 is the only one in the list)
  widget_.UpdateList({test_process_info_1_});
  ASSERT_GE(selected_spy.count(), 1);
  argument = selected_spy.takeFirst().at(0);
  ASSERT_TRUE(argument.canConvert<ProcessInfo>());
  process_info = argument.value<ProcessInfo>();
  EXPECT_TRUE(
      google::protobuf::util::MessageDifferencer::Equals(process_info, test_process_info_1_));
}

TEST_F(ProcessListWidgetTest, SetNameToSelect) {
  QSignalSpy selected_spy(&widget_, &ProcessListWidget::ProcessSelected);

  widget_.SetProcessNameToSelect(test_process_info_1_.name());

  widget_.UpdateList({test_process_info_1_, test_process_info_2_});
  ASSERT_EQ(selected_spy.count(), 1);
  QVariant argument = selected_spy.takeFirst().at(0);
  ASSERT_TRUE(argument.canConvert<ProcessInfo>());
  auto process_info = argument.value<ProcessInfo>();
  EXPECT_TRUE(
      google::protobuf::util::MessageDifferencer::Equals(process_info, test_process_info_1_));
}

TEST_F(ProcessListWidgetTest, NoSelection) {
  QSignalSpy no_selection_spy(&widget_, &ProcessListWidget::ProcessSelectionCleared);

  // setup, so there is a selection
  widget_.SetProcessNameToSelect(test_process_info_1_.name());
  widget_.UpdateList({test_process_info_1_});
  EXPECT_EQ(table_view_->model()->rowCount(), 1);

  // deselect on clear
  widget_.Clear();
  EXPECT_EQ(no_selection_spy.count(), 1);

  no_selection_spy.clear();

  // setup, so there is a selection
  widget_.SetProcessNameToSelect(test_process_info_1_.name());
  widget_.UpdateList({test_process_info_1_});
  EXPECT_EQ(table_view_->model()->rowCount(), 1);

  // deselect when table is empty because of filtering
  filter_line_edit_->setText("filter string that does not create match");
  EXPECT_EQ(table_view_->model()->rowCount(), 0);
  EXPECT_EQ(no_selection_spy.count(), 1);
}

TEST_F(ProcessListWidgetTest, UpdateList) {
  EXPECT_EQ(table_view_->model()->rowCount(), 0);

  widget_.UpdateList({test_process_info_1_});
  EXPECT_EQ(table_view_->model()->rowCount(), 1);

  widget_.UpdateList({test_process_info_1_});
  EXPECT_EQ(table_view_->model()->rowCount(), 1);

  widget_.UpdateList({test_process_info_1_, test_process_info_2_});
  EXPECT_EQ(table_view_->model()->rowCount(), 2);

  widget_.UpdateList({test_process_info_2_});
  EXPECT_EQ(table_view_->model()->rowCount(), 1);
}

TEST_F(ProcessListWidgetTest, Confirmed) {
  QSignalSpy confirmed_spy(&widget_, &ProcessListWidget::ProcessConfirmed);

  // no confirm via enter in filter line, iff a selection hasn't happened yet.
  QTest::keyClick(filter_line_edit_, Qt::Key::Key_Enter);
  EXPECT_EQ(confirmed_spy.count(), 0);

  // make auto selection happen
  QSignalSpy selected_spy(&widget_, &ProcessListWidget::ProcessSelected);
  widget_.UpdateList({test_process_info_1_});
  widget_.UpdateList({test_process_info_1_});
  EXPECT_EQ(selected_spy.count(), 1);

  // no confirm yet
  EXPECT_EQ(confirmed_spy.count(), 0);

  // confirm via enter
  QTest::keyClick(filter_line_edit_, Qt::Key::Key_Enter);
  EXPECT_EQ(confirmed_spy.count(), 1);

  {  // confirm via double click

    // selection_row_box is the rectangle that the selected row is occupying. These coordinates are
    // relative to the viewport of the table view (table_view_->viewport()).
    const QRect selected_row_box =
        table_view_->visualRect(table_view_->selectionModel()->currentIndex());

    // The following does a single mouse click onto the selected row (before the double click
    // later). This is only necessary when simulating the click via QTest and not when an actual
    // human is double clicking. The reason for this is not clear to me. Some more information can
    // be found here:
    // https://stackoverflow.com/questions/12604739/how-can-you-edit-a-qtableview-cell-from-a-qtest-unit-test
    QTest::mouseClick(table_view_->viewport(), Qt::MouseButton::LeftButton,
                      Qt::KeyboardModifier::NoModifier, selected_row_box.center());

    QTest::mouseDClick(table_view_->viewport(), Qt::MouseButton::LeftButton,
                       Qt::KeyboardModifier::NoModifier, selected_row_box.center());
    EXPECT_EQ(confirmed_spy.count(), 2);
  }
}

}  // namespace orbit_session_setup