// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include "OrbitGgp/Client.h"
#include "OrbitGgp/Project.h"
#include "SessionSetup/RetrieveInstancesWidget.h"

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using orbit_ggp::SshInfo;

class MockGgpClient : public orbit_ggp::Client {
 public:
  MOCK_METHOD(Future<ErrorMessageOr<QVector<Instance>>>, GetInstancesAsync,
              (bool /*all_reserved*/, std::optional<Project> /*project*/), (override));
  MOCK_METHOD(Future<ErrorMessageOr<QVector<Instance>>>, GetInstancesAsync,
              (bool /*all_reserved*/, std::optional<Project> /*project*/, int /*retry*/),
              (override));
  MOCK_METHOD(Future<ErrorMessageOr<SshInfo>>, GetSshInfoAsync,
              (const Instance& /*ggp_instance*/, std::optional<Project> /*project*/), (override));
  MOCK_METHOD(Future<ErrorMessageOr<QVector<Project>>>, GetProjectsAsync, (), (override));
  MOCK_METHOD(Future<ErrorMessageOr<Project>>, GetDefaultProjectAsync, (), (override));
};

namespace {

class RetrieveInstancesWidgetTest : public testing::Test {
 public:
  RetrieveInstancesWidgetTest() : widget_(&mock_ggp_) {
    filter_line_edit_ = widget_.findChild<QLineEdit*>("filterLineEdit");
    all_check_box_ = widget_.findChild<QCheckBox*>("allCheckBox");
    project_combo_box_ = widget_.findChild<QComboBox*>("projectComboBox");
    reload_button_ = widget_.findChild<QPushButton*>("reloadButton");
  }

 protected:
  void SetUp() override {
    ASSERT_NE(filter_line_edit_, nullptr);
    ASSERT_NE(all_check_box_, nullptr);
    ASSERT_NE(project_combo_box_, nullptr);
    ASSERT_NE(reload_button_, nullptr);
  }
  MockGgpClient mock_ggp_;
  RetrieveInstancesWidget widget_;
  QLineEdit* filter_line_edit_ = nullptr;
  QCheckBox* all_check_box_ = nullptr;
  QComboBox* project_combo_box_ = nullptr;
  QPushButton* reload_button_ = nullptr;
};

}  // namespace

TEST_F(RetrieveInstancesWidgetTest, FilterTextChanged) {
  QSignalSpy spy{&widget_, &RetrieveInstancesWidget::FilterTextChanged};

  QTest::keyClicks(filter_line_edit_, "test text");

  EXPECT_EQ(spy.count(), 9);  // 9 chars in "test text"

  QList<QVariant> arguments = spy.takeLast();
  ASSERT_EQ(arguments.count(), 1);
  ASSERT_TRUE(arguments.at(0).canConvert<QString>());

  EXPECT_EQ(arguments.at(0).toString(), "test text");
}

}  // namespace orbit_session_setup