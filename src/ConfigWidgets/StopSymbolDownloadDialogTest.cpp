// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QString>
#include <QTest>
#include <Qt>
#include <string>

#include "ClientData/ModuleData.h"
#include "ConfigWidgets/StopSymbolDownloadDialog.h"
#include "GrpcProtos/module.pb.h"

namespace orbit_config_widgets {

constexpr const char* kFilePath = "test/file/path";

orbit_grpc_protos::ModuleInfo CreateModuleInfo() {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_file_path(kFilePath);
  return module_info;
}

class StopSymbolDownloadDialogTest : public testing::Test {
 public:
  explicit StopSymbolDownloadDialogTest() : module_(CreateModuleInfo()), dialog_(&module_) {}

 protected:
  void SetUp() override {
    stop_button_ = dialog_.findChild<QPushButton*>("stopDownloadButton");
    ASSERT_NE(stop_button_, nullptr);
  }
  orbit_client_data::ModuleData module_;
  StopSymbolDownloadDialog dialog_;
  QPushButton* stop_button_ = nullptr;
};

TEST_F(StopSymbolDownloadDialogTest, UiContent) {
  auto* module_label = dialog_.findChild<QLabel*>("moduleLabel");
  ASSERT_NE(module_label, nullptr);
  std::string module_label_text = module_label->text().toStdString();
  EXPECT_TRUE(absl::StrContains(module_label_text, kFilePath));
}

TEST_F(StopSymbolDownloadDialogTest, Canceled) {
  auto* cancel_button = dialog_.findChild<QPushButton*>("cancelButton");
  ASSERT_NE(cancel_button, nullptr);

  QMetaObject::invokeMethod(
      &dialog_, [&] { QTest::mouseClick(cancel_button, Qt::LeftButton); }, Qt::QueuedConnection);
  StopSymbolDownloadDialog::Result dialog_result = dialog_.Exec();

  EXPECT_EQ(dialog_result, StopSymbolDownloadDialog::Result::kCancel);
}

TEST_F(StopSymbolDownloadDialogTest, StopDownload) {
  QMetaObject::invokeMethod(
      &dialog_, [this] { QTest::mouseClick(stop_button_, Qt::LeftButton); }, Qt::QueuedConnection);
  StopSymbolDownloadDialog::Result dialog_result = dialog_.Exec();

  EXPECT_EQ(dialog_result, StopSymbolDownloadDialog::Result::kStop);
}

TEST_F(StopSymbolDownloadDialogTest, StopDownloadAndDisable) {
  auto* remember_check_box = dialog_.findChild<QCheckBox*>("rememberCheckBox");
  ASSERT_NE(remember_check_box, nullptr);

  remember_check_box->setCheckState(Qt::Checked);

  QMetaObject::invokeMethod(
      &dialog_, [this] { QTest::mouseClick(stop_button_, Qt::LeftButton); }, Qt::QueuedConnection);
  StopSymbolDownloadDialog::Result dialog_result = dialog_.Exec();

  EXPECT_EQ(dialog_result, StopSymbolDownloadDialog::Result::kStopAndDisable);
}

}  // namespace orbit_config_widgets