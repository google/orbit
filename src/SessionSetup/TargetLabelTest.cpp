// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QColor>
#include <QString>
#include <filesystem>
#include <memory>
#include <optional>

#include "ClientData/ProcessData.h"
#include "GrpcProtos/process.pb.h"
#include "SessionSetup/TargetLabel.h"

namespace orbit_session_setup {

TEST(TargetLabel, Constructor) {
  TargetLabel label{};
  EXPECT_TRUE(label.GetTargetText().isEmpty());
  EXPECT_TRUE(label.GetFileText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_FALSE(label.GetIconType().has_value());
  EXPECT_FALSE(label.GetFilePath().has_value());
}

TEST(TargetLabel, ChangeToFileTarget) {
  TargetLabel label{};

  const std::filesystem::path filename = "file.orbit";
  const std::filesystem::path path = "dummy/path/" / filename;

  label.ChangeToFileTarget(path);

  EXPECT_EQ(label.GetFileText().toStdString(), filename);
  EXPECT_TRUE(label.GetTargetText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_FALSE(label.GetIconType().has_value());
  ASSERT_TRUE(label.GetFilePath().has_value());
  EXPECT_EQ(label.GetFilePath().value(), path);
}

const char* kProcessName = "test process";
const char* kInstanceName = "test instance";
const double kCpuUsage = 50.1;
const char* kCpuUsageDisplay = "50%";
const char* kSshMachineId = "1.1.1.1:2222";

static void ChangeToFakeSshTarget(TargetLabel& label) {
  orbit_client_data::ProcessData process;
  orbit_grpc_protos::ProcessInfo process_info;
  process_info.set_name(kProcessName);
  process_info.set_full_path("/mnt/developer/test_process");
  process_info.set_cpu_usage(kCpuUsage);
  process.SetProcessInfo(process_info);

  label.ChangeToSshTarget(process, kSshMachineId);
}

TEST(TargetLabel, ChangeToSshTarget) {
  TargetLabel label{};
  const QColor initial_color = label.GetTargetColor();

  ChangeToFakeSshTarget(label);

  EXPECT_EQ(label.GetTargetText(),
            QString{"%1 (%2) @ %3"}.arg(kProcessName, kCpuUsageDisplay, kSshMachineId));
  EXPECT_TRUE(label.GetFileText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().contains(kProcessName));
  EXPECT_TRUE(label.GetToolTip().contains(kSshMachineId));
  EXPECT_NE(label.GetTargetColor(), initial_color);
  ASSERT_TRUE(label.GetIconType().has_value());
  EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
  EXPECT_FALSE(label.GetFilePath().has_value());
}

TEST(TargetLabel, ChangeToLocalTarget) {
  TargetLabel label{};
  const QColor initial_color = label.GetTargetColor();

  const QString process_name = "test process";
  const double cpu_usage = 50.1;

  label.ChangeToLocalTarget(process_name, cpu_usage);

  EXPECT_EQ(label.GetTargetText(), "test process (50%) @ localhost");
  EXPECT_TRUE(label.GetFileText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_NE(label.GetTargetColor(), initial_color);
  ASSERT_TRUE(label.GetIconType().has_value());
  EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
  EXPECT_FALSE(label.GetFilePath().has_value());
}

TEST(TargetLabel, SetProcessCpuUsageInPercent) {
  TargetLabel label({});
  const QString initial_target_text = label.GetTargetText();
  const QString initial_file_text = label.GetFileText();
  const QString initial_tool_tip = label.GetToolTip();
  const QColor initial_color = label.GetTargetColor();

  {
    const bool result = label.SetProcessCpuUsageInPercent(20);
    EXPECT_FALSE(result);
    EXPECT_EQ(label.GetTargetText(), initial_target_text);
    EXPECT_EQ(label.GetFileText(), initial_file_text);
    EXPECT_EQ(label.GetToolTip(), initial_tool_tip);
    EXPECT_EQ(label.GetTargetColor(), initial_color);
    EXPECT_FALSE(label.GetIconType().has_value());
    EXPECT_FALSE(label.GetFilePath().has_value());
  }

  label.ChangeToLocalTarget("test", 10.2);
  const QString updated_target_text = label.GetTargetText();
  EXPECT_NE(updated_target_text, initial_target_text);
  EXPECT_EQ(label.GetFileText(), initial_file_text);
  const QColor updated_color = label.GetTargetColor();
  EXPECT_NE(updated_color, initial_color);
  {
    const bool result = label.SetProcessCpuUsageInPercent(20.7);
    EXPECT_TRUE(result);
    EXPECT_NE(label.GetTargetText(), updated_target_text);
    EXPECT_TRUE(label.GetTargetText().contains("21%"));
    EXPECT_EQ(label.GetFileText(), initial_file_text);
    EXPECT_TRUE(label.GetToolTip().isEmpty());
    EXPECT_EQ(label.GetTargetColor(), updated_color);
    ASSERT_TRUE(label.GetIconType().has_value());
    EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
    EXPECT_FALSE(label.GetFilePath().has_value());
  }
}

TEST(TargetLabel, SetProcessEnded) {
  TargetLabel label({});
  const QString initial_target_text = label.GetTargetText();
  const QString initial_file_text = label.GetFileText();
  const QString initial_tool_tip = label.GetToolTip();
  const QColor initial_color = label.GetTargetColor();

  {
    const bool result = label.SetProcessEnded();
    EXPECT_FALSE(result);
    EXPECT_EQ(label.GetTargetText(), initial_target_text);
    EXPECT_EQ(label.GetFileText(), initial_file_text);
    EXPECT_EQ(label.GetToolTip(), initial_tool_tip);
    EXPECT_EQ(label.GetTargetColor(), initial_color);
    EXPECT_FALSE(label.GetIconType().has_value());
    EXPECT_FALSE(label.GetFilePath().has_value());
  }

  label.ChangeToLocalTarget("test", 10.2);
  const QString updated_target_text = label.GetTargetText();
  EXPECT_NE(updated_target_text, initial_target_text);
  const QColor updated_color = label.GetTargetColor();
  EXPECT_NE(updated_color, initial_color);
  {
    const bool result = label.SetProcessEnded();
    EXPECT_TRUE(result);
    EXPECT_NE(label.GetTargetText(), updated_target_text);
    EXPECT_FALSE(label.GetTargetText().contains("10%"));
    EXPECT_EQ(label.GetToolTip(), "The process ended.");
    EXPECT_EQ(label.GetFileText(), initial_file_text);
    EXPECT_NE(label.GetTargetColor(), initial_color);
    EXPECT_NE(label.GetTargetColor(), updated_color);
    ASSERT_TRUE(label.GetIconType().has_value());
    EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kOrangeDisconnectedIcon);
    EXPECT_FALSE(label.GetFilePath().has_value());
  }
}

TEST(TargetLabel, SetConnectionDead) {
  TargetLabel label({});
  const QString initial_target_text = label.GetTargetText();
  const QString initial_file_text = label.GetFileText();
  const QString initial_tool_tip = label.GetToolTip();
  const QColor initial_color = label.GetTargetColor();

  {
    const bool result = label.SetConnectionDead("");
    EXPECT_FALSE(result);
    EXPECT_EQ(label.GetTargetText(), initial_target_text);
    EXPECT_EQ(label.GetFileText(), initial_file_text);
    EXPECT_EQ(label.GetToolTip(), initial_tool_tip);
    EXPECT_EQ(label.GetTargetColor(), initial_color);
    EXPECT_FALSE(label.GetIconType().has_value());
    EXPECT_FALSE(label.GetFilePath().has_value());
  }

  label.ChangeToLocalTarget("test", 10.2);
  const QString updated_target_text = label.GetTargetText();
  EXPECT_NE(updated_target_text, initial_target_text);
  EXPECT_EQ(label.GetFileText(), initial_file_text);
  const QColor updated_color = label.GetTargetColor();
  EXPECT_NE(updated_color, initial_color);

  const QString error_message = "test error message";
  {
    const bool result = label.SetConnectionDead(error_message);
    EXPECT_TRUE(result);
    EXPECT_NE(label.GetTargetText(), updated_target_text);
    EXPECT_FALSE(label.GetTargetText().contains("10%"));
    EXPECT_EQ(label.GetFileText(), initial_file_text);
    EXPECT_EQ(label.GetToolTip(), error_message);
    EXPECT_NE(label.GetTargetColor(), initial_color);
    EXPECT_NE(label.GetTargetColor(), updated_color);
    ASSERT_TRUE(label.GetIconType().has_value());
    EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kRedDisconnectedIcon);
    EXPECT_FALSE(label.GetFilePath().has_value());
  }
}

TEST(TargetLabel, SetFile) {
  TargetLabel label{};
  const QColor initial_color = label.GetTargetColor();

  ChangeToFakeSshTarget(label);

  EXPECT_EQ(label.GetTargetText(),
            QString{"%1 (%2) @ %3"}.arg(kProcessName, kCpuUsageDisplay, kSshMachineId));
  EXPECT_TRUE(label.GetFileText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().contains(kProcessName));
  EXPECT_TRUE(label.GetToolTip().contains(kSshMachineId));
  EXPECT_NE(label.GetTargetColor(), initial_color);
  ASSERT_TRUE(label.GetIconType().has_value());
  EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
  EXPECT_FALSE(label.GetFilePath().has_value());

  const std::filesystem::path path{"/some/file"};
  label.SetFile(path);

  EXPECT_EQ(label.GetTargetText(),
            QString{"%1 (%2) @ %3"}.arg(kProcessName, kCpuUsageDisplay, kSshMachineId));
  EXPECT_EQ(label.GetFileText(), "file");
  EXPECT_TRUE(label.GetToolTip().contains(kProcessName));
  EXPECT_TRUE(label.GetToolTip().contains(kSshMachineId));
  EXPECT_NE(label.GetTargetColor(), initial_color);
  ASSERT_TRUE(label.GetIconType().has_value());
  EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
  ASSERT_TRUE(label.GetFilePath().has_value());
  EXPECT_EQ(label.GetFilePath().value(), path);
}

TEST(TargetLabel, Clear) {
  TargetLabel label;

  label.ChangeToLocalTarget("test", 10.2);
  label.SetFile("/some/file");
  label.SetProcessEnded();

  EXPECT_FALSE(label.GetTargetText().isEmpty());
  EXPECT_FALSE(label.GetToolTip().isEmpty());
  EXPECT_FALSE(label.GetFileText().isEmpty());

  const QColor ended_color = label.GetTargetColor();

  label.Clear();
  EXPECT_TRUE(label.GetTargetText().isEmpty());
  EXPECT_TRUE(label.GetFileText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_NE(label.GetTargetColor(), ended_color);
  EXPECT_FALSE(label.GetIconType().has_value());
  EXPECT_FALSE(label.GetFilePath().has_value());
}

TEST(TargetLabel, DifferentColors) {
  TargetLabel label{};

  label.ChangeToFileTarget("test/path");
  const QColor file_color = label.GetTargetColor();

  ChangeToFakeSshTarget(label);
  const QColor ssh_color = label.GetTargetColor();

  label.ChangeToLocalTarget("test process", 0);
  const QColor local_color = label.GetTargetColor();

  label.SetProcessCpuUsageInPercent(10);
  const QColor cpu_usage_updated_color = label.GetTargetColor();

  label.SetProcessEnded();
  const QColor process_ended_color = label.GetTargetColor();

  label.SetConnectionDead("test error");
  const QColor connection_dead_color = label.GetTargetColor();

  EXPECT_EQ(ssh_color, local_color);
  EXPECT_EQ(ssh_color, cpu_usage_updated_color);

  EXPECT_NE(file_color, ssh_color);
  EXPECT_NE(file_color, process_ended_color);
  EXPECT_NE(file_color, connection_dead_color);

  EXPECT_NE(ssh_color, process_ended_color);
  EXPECT_NE(ssh_color, connection_dead_color);

  EXPECT_NE(process_ended_color, connection_dead_color);
}

}  // namespace orbit_session_setup