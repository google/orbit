// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "TargetLabel.h"
#include "process.pb.h"

namespace orbit_qt {

TEST(TargetLabel, Constructor) {
  TargetLabel label{};
  EXPECT_TRUE(label.GetText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_FALSE(label.GetIconType().has_value());
}

TEST(TargetLabel, ChangeToFileTarget) {
  TargetLabel label{};

  const std::filesystem::path filename = "file.orbit";
  const std::filesystem::path path = "dummy/path/" / filename;

  label.ChangeToFileTarget(path);

  EXPECT_EQ(label.GetText().toStdString(), filename);
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_FALSE(label.GetIconType().has_value());
}

TEST(TargetLabel, ChangeToStadiaTarget) {
  TargetLabel label{};
  const QColor initial_color = label.GetColor();

  const QString process_name = "test process";
  const double cpu_usage = 50.1;
  const QString instance_name = "test instance";

  label.ChangeToStadiaTarget(process_name, cpu_usage, instance_name);

  EXPECT_EQ(label.GetText(), "test process (50%) @ test instance");
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_NE(label.GetColor(), initial_color);
  ASSERT_TRUE(label.GetIconType().has_value());
  EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
}

TEST(TargetLabel, ChangeToLocalTarget) {
  TargetLabel label{};
  const QColor initial_color = label.GetColor();

  const QString process_name = "test process";
  const double cpu_usage = 50.1;

  label.ChangeToLocalTarget(process_name, cpu_usage);

  EXPECT_EQ(label.GetText(), "test process (50%) @ localhost");
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_NE(label.GetColor(), initial_color);
  ASSERT_TRUE(label.GetIconType().has_value());
  EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
}

TEST(TargetLabel, SetProcessCpuUsageInPercent) {
  TargetLabel label({});
  const QString initial_text = label.GetText();
  const QString initial_tool_tip = label.GetToolTip();
  const QColor initial_color = label.GetColor();

  {
    const bool result = label.SetProcessCpuUsageInPercent(20);
    EXPECT_FALSE(result);
    EXPECT_EQ(label.GetText(), initial_text);
    EXPECT_EQ(label.GetToolTip(), initial_tool_tip);
    EXPECT_EQ(label.GetColor(), initial_color);
    EXPECT_FALSE(label.GetIconType().has_value());
  }

  label.ChangeToLocalTarget("test", 10.2);
  const QString updated_text = label.GetText();
  EXPECT_NE(updated_text, initial_text);
  const QColor updated_color = label.GetColor();
  EXPECT_NE(updated_color, initial_color);
  {
    const bool result = label.SetProcessCpuUsageInPercent(20.7);
    EXPECT_TRUE(result);
    EXPECT_NE(label.GetText(), updated_text);
    EXPECT_TRUE(label.GetText().contains("21%"));
    EXPECT_TRUE(label.GetToolTip().isEmpty());
    EXPECT_EQ(label.GetColor(), updated_color);
    ASSERT_TRUE(label.GetIconType().has_value());
    EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kGreenConnectedIcon);
  }
}

TEST(TargetLabel, SetProcessEnded) {
  TargetLabel label({});
  const QString initial_text = label.GetText();
  const QString initial_tool_tip = label.GetToolTip();
  const QColor initial_color = label.GetColor();

  {
    const bool result = label.SetProcessEnded();
    EXPECT_FALSE(result);
    EXPECT_EQ(label.GetText(), initial_text);
    EXPECT_EQ(label.GetToolTip(), initial_tool_tip);
    EXPECT_EQ(label.GetColor(), initial_color);
    EXPECT_FALSE(label.GetIconType().has_value());
  }

  label.ChangeToLocalTarget("test", 10.2);
  const QString updated_text = label.GetText();
  EXPECT_NE(updated_text, initial_text);
  const QColor updated_color = label.GetColor();
  EXPECT_NE(updated_color, initial_color);
  {
    const bool result = label.SetProcessEnded();
    EXPECT_TRUE(result);
    EXPECT_NE(label.GetText(), updated_text);
    EXPECT_FALSE(label.GetText().contains("10%"));
    EXPECT_EQ(label.GetToolTip(), "The process ended. Restart the process to continue profiling.");
    EXPECT_NE(label.GetColor(), initial_color);
    EXPECT_NE(label.GetColor(), updated_color);
    ASSERT_TRUE(label.GetIconType().has_value());
    EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kOrangeDisconnectedIcon);
  }
}

TEST(TargetLabel, SetConnectionDead) {
  TargetLabel label({});
  const QString initial_text = label.GetText();
  const QString initial_tool_tip = label.GetToolTip();
  const QColor initial_color = label.GetColor();

  {
    const bool result = label.SetConnectionDead("");
    EXPECT_FALSE(result);
    EXPECT_EQ(label.GetText(), initial_text);
    EXPECT_EQ(label.GetToolTip(), initial_tool_tip);
    EXPECT_EQ(label.GetColor(), initial_color);
    EXPECT_FALSE(label.GetIconType().has_value());
  }

  label.ChangeToLocalTarget("test", 10.2);
  const QString updated_text = label.GetText();
  EXPECT_NE(updated_text, initial_text);
  const QColor updated_color = label.GetColor();
  EXPECT_NE(updated_color, initial_color);

  const QString error_message = "test error message";
  {
    const bool result = label.SetConnectionDead(error_message);
    EXPECT_TRUE(result);
    EXPECT_NE(label.GetText(), updated_text);
    EXPECT_FALSE(label.GetText().contains("10%"));
    EXPECT_EQ(label.GetToolTip(), error_message);
    EXPECT_NE(label.GetColor(), initial_color);
    EXPECT_NE(label.GetColor(), updated_color);
    ASSERT_TRUE(label.GetIconType().has_value());
    EXPECT_EQ(label.GetIconType().value(), TargetLabel::IconType::kRedDisconnectedIcon);
  }
}

TEST(TargetLabel, Clear) {
  TargetLabel label;

  label.ChangeToLocalTarget("test", 10.2);
  label.SetProcessEnded();

  EXPECT_FALSE(label.GetText().isEmpty());
  EXPECT_FALSE(label.GetToolTip().isEmpty());

  const QColor ended_color = label.GetColor();

  label.Clear();
  EXPECT_TRUE(label.GetText().isEmpty());
  EXPECT_TRUE(label.GetToolTip().isEmpty());
  EXPECT_NE(label.GetColor(), ended_color);
  ASSERT_FALSE(label.GetIconType().has_value());
}

TEST(TargetLabel, DifferentColors) {
  TargetLabel label{};

  label.ChangeToFileTarget("test/path");
  const QColor file_color = label.GetColor();

  label.ChangeToStadiaTarget("test process", 0, "test instance");
  const QColor stadia_color = label.GetColor();

  label.ChangeToLocalTarget("test process", 0);
  const QColor local_color = label.GetColor();

  label.SetProcessCpuUsageInPercent(10);
  const QColor cpu_usage_updated_color = label.GetColor();

  label.SetProcessEnded();
  const QColor process_ended_color = label.GetColor();

  label.SetConnectionDead("test error");
  const QColor connection_dead_color = label.GetColor();

  EXPECT_EQ(stadia_color, local_color);
  EXPECT_EQ(stadia_color, cpu_usage_updated_color);

  EXPECT_NE(file_color, stadia_color);
  EXPECT_NE(file_color, process_ended_color);
  EXPECT_NE(file_color, connection_dead_color);

  EXPECT_NE(stadia_color, process_ended_color);
  EXPECT_NE(stadia_color, connection_dead_color);

  EXPECT_NE(process_ended_color, connection_dead_color);
}

}  // namespace orbit_qt