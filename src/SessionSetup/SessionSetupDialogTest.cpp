// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <QMetaObject>
#include <QPushButton>
#include <QString>
#include <QTest>
#include <Qt>
#include <filesystem>
#include <memory>
#include <optional>
#include <outcome.hpp>
#include <utility>
#include <variant>

#include "OrbitSsh/Context.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/DeploymentConfigurations.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SessionSetup/SessionSetupDialog.h"
#include "SessionSetup/TargetConfiguration.h"
#include "SessionSetup/TargetLabel.h"

namespace orbit_session_setup {

namespace {

class SessionSetupDialogTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto ssh_context = orbit_ssh::Context::Create();
    ASSERT_TRUE(ssh_context.has_value());
    ssh_context_ = std::make_unique<orbit_ssh::Context>(std::move(ssh_context.value()));
    ssh_artifacts_ = std::make_unique<SshConnectionArtifacts>(
        ssh_context_.get(), ServiceDeployManager::GrpcPort{0}, &deploy_config_);
  }
  std::unique_ptr<orbit_ssh::Context> ssh_context_;
  DeploymentConfiguration deploy_config_{NoDeployment{}};
  std::unique_ptr<SshConnectionArtifacts> ssh_artifacts_;
};

}  // namespace

TEST_F(SessionSetupDialogTest, CreateExecAndRejectEmptyDialogueReturnsNoConfiguration) {
  SessionSetupDialog dialog{ssh_artifacts_.get(), std::nullopt};

  EXPECT_TRUE(dialog.isEnabled());

  QMetaObject::invokeMethod(
      &dialog,
      [&dialog]() {
        QApplication::processEvents();
        auto* confirm_button = dialog.findChild<QPushButton*>("confirmButton");
        ASSERT_NE(confirm_button, nullptr);
        EXPECT_FALSE(confirm_button->isEnabled());

        dialog.reject();
      },
      Qt::ConnectionType::QueuedConnection);

  std::optional<TargetConfiguration> result = dialog.Exec();
  EXPECT_EQ(result, std::nullopt);
}

TEST_F(SessionSetupDialogTest, CreateExecAndStartDialogueWithFileTargetReturnsValidConfiguration) {
  const std::filesystem::path file_path = "test/path/to/file";
  FileTarget file_target{file_path};

  SessionSetupDialog dialog(ssh_artifacts_.get(), file_target);

  QMetaObject::invokeMethod(
      &dialog,
      [&]() {
        QApplication::processEvents();
        auto* target_label = dialog.findChild<TargetLabel*>("targetLabel");
        ASSERT_NE(target_label, nullptr);
        EXPECT_EQ(target_label->GetFileText().toStdString(), file_path.filename().string());

        auto* confirm_button = dialog.findChild<QPushButton*>("confirmButton");
        ASSERT_NE(confirm_button, nullptr);
        EXPECT_TRUE(confirm_button->isEnabled());

        QTest::mouseClick(confirm_button, Qt::MouseButton::LeftButton);
      },
      Qt::ConnectionType::QueuedConnection);

  std::optional<TargetConfiguration> result = dialog.Exec();
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(std::holds_alternative<FileTarget>(result.value()));
  const FileTarget& result_file_target{std::get<FileTarget>(result.value())};
  EXPECT_EQ(result_file_target.GetCaptureFilePath(), file_path);
}

}  // namespace orbit_session_setup