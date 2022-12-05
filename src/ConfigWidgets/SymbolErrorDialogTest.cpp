// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QDialog>
#include <QLabel>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QString>
#include <QTest>
#include <Qt>
#include <string>
#include <string_view>
#include <utility>

#include "ClientData/ModuleData.h"
#include "ConfigWidgets/SymbolErrorDialog.h"
#include "GrpcProtos/module.pb.h"

namespace orbit_config_widgets {

orbit_grpc_protos::ModuleInfo CreateModuleInfo(std::string module_path) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_file_path(std::move(module_path));
  module_info.set_build_id("some build id");
  return module_info;
}
class SymbolErrorDialogTest : public testing::Test {
 public:
  SymbolErrorDialogTest() : module_{CreateModuleInfo(module_path_)}, dialog_(&module_, error_) {}

 protected:
  void SetUp() override {
    error_plain_text_edit_ = dialog_.findChild<QPlainTextEdit*>("errorPlainTextEdit");
    ASSERT_NE(error_plain_text_edit_, nullptr);
    show_error_button_ = dialog_.findChild<QPushButton*>("showErrorButton");
    ASSERT_NE(show_error_button_, nullptr);
  }

  const std::string module_path_ = "example/file/path/module.so";
  const std::string error_ = "example error text";
  const orbit_client_data::ModuleData module_;
  SymbolErrorDialog dialog_;
  QPlainTextEdit* error_plain_text_edit_ = nullptr;
  QPushButton* show_error_button_ = nullptr;
};

TEST_F(SymbolErrorDialogTest, UiElements) {
  auto* module_name_label = dialog_.findChild<QLabel*>("moduleNameLabel");
  ASSERT_NE(module_name_label, nullptr);
  EXPECT_EQ(module_name_label->text().toStdString(), module_path_);

  EXPECT_EQ(error_plain_text_edit_->toPlainText().toStdString(), error_);
  EXPECT_FALSE(error_plain_text_edit_->isVisible());

  EXPECT_EQ(show_error_button_->text().toStdString(), "Show detailed error");
}

TEST_F(SymbolErrorDialogTest, OnShowErrorButtonClicked) {
  EXPECT_FALSE(error_plain_text_edit_->isVisible());

  QMetaObject::invokeMethod(
      &dialog_,
      [&]() {
        QTest::mouseClick(show_error_button_, Qt::LeftButton);
        EXPECT_TRUE(error_plain_text_edit_->isVisible());
        EXPECT_EQ(show_error_button_->text().toStdString(), "Hide detailed error");
        dialog_.reject();
      },
      Qt::QueuedConnection);

  (void)dialog_.Exec();
}

TEST_F(SymbolErrorDialogTest, OnAddSymbolLocationButtonClicked) {
  auto* on_add_symbol_location_button = dialog_.findChild<QPushButton*>("addSymbolLocationButton");
  ASSERT_NE(on_add_symbol_location_button, nullptr);
  ASSERT_TRUE(on_add_symbol_location_button->isEnabled());
  QMetaObject::invokeMethod(
      &dialog_, [&]() { QTest::mouseClick(on_add_symbol_location_button, Qt::LeftButton); },
      Qt::QueuedConnection);

  SymbolErrorDialog::Result result = dialog_.Exec();
  EXPECT_EQ(result, SymbolErrorDialog::Result::kAddSymbolLocation);
}

TEST_F(SymbolErrorDialogTest, OnTryAgainButtonClicked) {
  auto* on_try_again_button = dialog_.findChild<QPushButton*>("tryAgainButton");
  ASSERT_NE(on_try_again_button, nullptr);
  QMetaObject::invokeMethod(
      &dialog_, [&]() { QTest::mouseClick(on_try_again_button, Qt::LeftButton); },
      Qt::QueuedConnection);

  SymbolErrorDialog::Result result = dialog_.Exec();
  EXPECT_EQ(result, SymbolErrorDialog::Result::kTryAgain);
}

TEST_F(SymbolErrorDialogTest, OnCancelButtonClicked) {
  auto* on_cancel_button = dialog_.findChild<QPushButton*>("cancelButton");
  ASSERT_NE(on_cancel_button, nullptr);
  QMetaObject::invokeMethod(
      &dialog_, [&]() { QTest::mouseClick(on_cancel_button, Qt::LeftButton); },
      Qt::QueuedConnection);

  SymbolErrorDialog::Result result = dialog_.Exec();
  EXPECT_EQ(result, SymbolErrorDialog::Result::kCancel);
}

TEST_F(SymbolErrorDialogTest, OnRejected) {
  QMetaObject::invokeMethod(&dialog_, &QDialog::reject, Qt::QueuedConnection);

  SymbolErrorDialog::Result result = dialog_.Exec();
  EXPECT_EQ(result, SymbolErrorDialog::Result::kCancel);
}

TEST(SymbolErrorDialog, EmptyBuildId) {
  orbit_client_data::ModuleData module{{}};
  SymbolErrorDialog dialog{&module, "error"};
  auto* on_add_symbol_location_button = dialog.findChild<QPushButton*>("addSymbolLocationButton");
  ASSERT_NE(on_add_symbol_location_button, nullptr);
  EXPECT_FALSE(on_add_symbol_location_button->isEnabled());
}

}  // namespace orbit_config_widgets