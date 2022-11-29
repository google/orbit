// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QTableView>
#include <QTest>
#include <QVariant>
#include <Qt>
#include <filesystem>
#include <memory>
#include <optional>

#include "CaptureFileInfo/Manager.h"
#include "SessionSetup/LoadCaptureWidget.h"
#include "Test/Path.h"

namespace orbit_session_setup {

constexpr const char* kOrgName = "The Orbit Authors";
using orbit_capture_file_info::Manager;

TEST(LoadCaptureWidget, RadioButton) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("LoadCaptureWidget.IsActiveSetActive");

  LoadCaptureWidget widget{};

  auto* radio_button = widget.findChild<QRadioButton*>("radioButton");
  ASSERT_TRUE(radio_button != nullptr);
  auto* capture_filter_line_edit = widget.findChild<QLineEdit*>("captureFilterLineEdit");
  ASSERT_TRUE(capture_filter_line_edit != nullptr);
  auto* select_file_button = widget.findChild<QPushButton*>("selectFileButton");
  ASSERT_TRUE(select_file_button != nullptr);
  auto* table_view = widget.findChild<QTableView*>("tableView");
  ASSERT_TRUE(table_view != nullptr);

  // radio button is always enabled
  EXPECT_TRUE(radio_button->isEnabled());
  EXPECT_FALSE(radio_button->isChecked());
  // default is disabled
  EXPECT_FALSE(capture_filter_line_edit->isEnabled());
  EXPECT_FALSE(select_file_button->isEnabled());
  EXPECT_FALSE(table_view->isEnabled());

  QTest::mouseClick(radio_button, Qt::LeftButton);
  // radio button is always enabled
  EXPECT_TRUE(radio_button->isEnabled());
  EXPECT_TRUE(radio_button->isChecked());
  // After click on radio button, ui is enabled
  EXPECT_TRUE(capture_filter_line_edit->isEnabled());
  EXPECT_TRUE(select_file_button->isEnabled());
  EXPECT_TRUE(table_view->isEnabled());
}

TEST(LoadCaptureWidget, SelectFromTableView) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("LoadCaptureWidget.SelectFromTableView");

  const std::filesystem::path test_file_path{orbit_test::GetTestdataDir() / "test_file.txt"};

  // To make sure there is one table entry, it is set here.
  Manager manager{};
  manager.Clear();
  manager.AddOrTouchCaptureFile(test_file_path, std::nullopt);

  LoadCaptureWidget widget{};
  auto* radio_button = widget.findChild<QRadioButton*>("radioButton");
  ASSERT_NE(radio_button, nullptr);
  // Enable the UI
  QTest::mouseClick(radio_button, Qt::LeftButton);

  auto* table_view = widget.findChild<QTableView*>("tableView");
  ASSERT_TRUE(table_view != nullptr);

  ASSERT_EQ(table_view->model()->rowCount(), 1);

  int x_pos = table_view->columnViewportPosition(0);
  int y_pos = table_view->rowViewportPosition(0);

  bool selection_happened = false;
  QObject::connect(&widget, &LoadCaptureWidget::FileSelected,
                   [&selection_happened, &test_file_path](const std::filesystem::path& file_path) {
                     EXPECT_EQ(file_path, test_file_path);
                     selection_happened = true;
                   });
  QTest::mouseClick(table_view->viewport(), Qt::MouseButton::LeftButton,
                    Qt::KeyboardModifier::NoModifier, QPoint{x_pos, y_pos});
  EXPECT_TRUE(selection_happened);

  bool confirm_happened = false;
  QObject::connect(&widget, &LoadCaptureWidget::SelectionConfirmed,
                   [&confirm_happened]() { confirm_happened = true; });
  QTest::mouseDClick(table_view->viewport(), Qt::MouseButton::LeftButton,
                     Qt::KeyboardModifier::NoModifier, QPoint{x_pos, y_pos});
  EXPECT_TRUE(confirm_happened);
}

TEST(LoadCaptureWidget, EditCaptureFileFilter) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("LoadCaptureWidget.EditCaptureFileFilter");

  const std::filesystem::path test_file_path0{orbit_test::GetTestdataDir() / "test_capture.orbit"};
  const std::filesystem::path test_file_path1{orbit_test::GetTestdataDir() / "test_file.txt"};

  Manager manager{};
  manager.Clear();
  manager.AddOrTouchCaptureFile(test_file_path0, std::nullopt);
  manager.AddOrTouchCaptureFile(test_file_path1, std::nullopt);

  LoadCaptureWidget widget{};
  auto* radio_button = widget.findChild<QRadioButton*>("radioButton");
  ASSERT_NE(radio_button, nullptr);
  // Enable the UI
  QTest::mouseClick(radio_button, Qt::LeftButton);

  auto* table_view = widget.findChild<QTableView*>("tableView");
  ASSERT_TRUE(table_view != nullptr);
  ASSERT_EQ(table_view->model()->rowCount(), 2);

  auto* capture_filter_line_edit = widget.findChild<QLineEdit*>("captureFilterLineEdit");
  QTest::keyClicks(capture_filter_line_edit, "cap");
  ASSERT_EQ(table_view->model()->rowCount(), 1);
  EXPECT_EQ(table_view->model()->index(0, 0).data().toString().toStdString(),
            test_file_path0.filename().string());

  QTest::keyClicks(capture_filter_line_edit, "123");
  ASSERT_EQ(table_view->model()->rowCount(), 0);
}

}  // namespace orbit_session_setup