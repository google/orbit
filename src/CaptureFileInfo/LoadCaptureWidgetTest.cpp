// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <QPushButton>
#include <QRadioButton>
#include <QTableView>
#include <QTest>
#include <filesystem>

#include "CaptureFileInfo/LoadCaptureWidget.h"
#include "CaptureFileInfo/Manager.h"
#include "OrbitBase/ExecutablePath.h"

namespace orbit_capture_file_info {

constexpr const char* kOrgName = "The Orbit Authors";

TEST(LoadCaptureWidget, IsActiveSetActive) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("LoadCaptureWidget.IsActiveSetActive");

  LoadCaptureWidget widget{};

  auto* radio_button = widget.findChild<QRadioButton*>("radioButton");
  ASSERT_TRUE(radio_button != nullptr);
  auto* select_file_button = widget.findChild<QPushButton*>("selectFileButton");
  ASSERT_TRUE(select_file_button != nullptr);
  auto* table_view = widget.findChild<QTableView*>("tableView");
  ASSERT_TRUE(table_view != nullptr);

  // The widget needs to be shown so that the detaching of the radio button takes place. Otherwise
  // the radioButton is not detached from the contentFrame and will be disabled when the widget is
  // set inactive.
  widget.show();
  QApplication::processEvents();

  // default is active
  EXPECT_TRUE(widget.IsActive());
  EXPECT_TRUE(select_file_button->isEnabled());
  EXPECT_TRUE(table_view->isEnabled());
  // radio button is always enabled
  EXPECT_TRUE(radio_button->isEnabled());

  // set active false
  widget.SetActive(false);
  EXPECT_FALSE(widget.IsActive());
  EXPECT_FALSE(select_file_button->isEnabled());
  EXPECT_FALSE(table_view->isEnabled());
  EXPECT_FALSE(radio_button->isChecked());
  // radio button is always enabled
  EXPECT_TRUE(radio_button->isEnabled());

  // set active true
  widget.SetActive(true);
  EXPECT_TRUE(widget.IsActive());
  EXPECT_TRUE(select_file_button->isEnabled());
  EXPECT_TRUE(table_view->isEnabled());
  EXPECT_TRUE(radio_button->isChecked());
  // radio button is always enabled
  EXPECT_TRUE(radio_button->isEnabled());

  // set active to false and then activate by clicking on the radiobutton
  widget.SetActive(false);
  bool signal_received = false;
  QObject::connect(&widget, &LoadCaptureWidget::Activated,
                   [&signal_received]() { signal_received = true; });
  QTest::mouseClick(radio_button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
  EXPECT_TRUE(signal_received);
}

TEST(LoadCaptureWidget, SelectFromTableView) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("LoadCaptureWidget.SelectFromTableView");

  const std::filesystem::path test_file_path{orbit_base::GetExecutableDir() / "testdata" /
                                             "CaptureFileInfo" / "test_file.txt"};

  // To make sure there is one table entry, it is set here.
  Manager manager{};
  manager.Clear();
  manager.AddOrTouchCaptureFile(test_file_path);

  LoadCaptureWidget widget{};
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

}  // namespace orbit_capture_file_info