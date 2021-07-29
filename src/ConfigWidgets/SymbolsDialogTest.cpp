// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QDialogButtonBox>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTest>
#include <filesystem>
#include <vector>

#include "ConfigWidgets/SymbolsDialog.h"

namespace orbit_config_widgets {

TEST(SymbolsDialog, GetSetSymbolPaths) {
  SymbolsDialog dialog;
  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");

  EXPECT_EQ(list_widget->count(), 0);
  EXPECT_EQ(dialog.GetSymbolPaths(), std::vector<std::filesystem::path>{});

  const std::vector<std::filesystem::path> test_paths{"/absolute/test/path1", "absolute/test/path2",
                                                      R"(C:\windows\test\path1)",
                                                      R"(C:\windows\test\path2)"};

  dialog.SetSymbolPaths(test_paths);
  EXPECT_EQ(list_widget->count(), test_paths.size());
  EXPECT_EQ(dialog.GetSymbolPaths(), test_paths);
}

TEST(SymbolsDialog, TryAddSymbolPath) {
  SymbolsDialog dialog;
  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  EXPECT_EQ(list_widget->count(), 0);

  // simple add succeeds
  std::filesystem::path path{"/absolute/test/path1"};
  dialog.TryAddSymbolPath(path);
  EXPECT_EQ(list_widget->count(), 1);
  EXPECT_EQ(dialog.GetSymbolPaths(), std::vector<std::filesystem::path>{path});

  // add the same path again -> warning that needs to be dismissed and nothing changes
  bool lambda_called = false;
  QMetaObject::invokeMethod(
      &dialog,
      [&]() {
        auto* message_box = dialog.findChild<QMessageBox*>();
        ASSERT_NE(message_box, nullptr);
        QAbstractButton* ok_button = message_box->button(QMessageBox::StandardButton::Ok);
        QTest::mouseClick(ok_button, Qt::MouseButton::LeftButton);
        lambda_called = true;
      },
      Qt::ConnectionType::QueuedConnection);
  dialog.TryAddSymbolPath(path);
  QApplication::processEvents();
  EXPECT_TRUE(lambda_called);
  EXPECT_EQ(list_widget->count(), 1);
  EXPECT_EQ(dialog.GetSymbolPaths(), std::vector<std::filesystem::path>{path});

  // add different path succeeds
  std::filesystem::path path2{R"(C:\windows\test\path1)"};
  dialog.TryAddSymbolPath(path2);
  EXPECT_EQ(list_widget->count(), 2);
  EXPECT_EQ(dialog.GetSymbolPaths(), (std::vector<std::filesystem::path>{path, path2}));
}

TEST(SymbolsDialog, Remove) {
  SymbolsDialog dialog;

  auto* remove_button = dialog.findChild<QPushButton*>("removeButton");
  EXPECT_FALSE(remove_button->isEnabled());

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");

  dialog.TryAddSymbolPath(R"(C:\windows\test\path1)");
  EXPECT_EQ(list_widget->count(), 1);
  list_widget->setCurrentRow(0);
  QApplication::processEvents();
  EXPECT_TRUE(remove_button->isEnabled());

  QTest::mouseClick(remove_button, Qt::MouseButton::LeftButton);

  EXPECT_EQ(list_widget->count(), 0);
  EXPECT_FALSE(remove_button->isEnabled());
}

}  // namespace orbit_config_widgets