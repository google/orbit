// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QRadioButton>
#include <QTest>
#include <Qt>
#include <memory>

#include "OrbitBase/Result.h"
#include "SessionSetup/ConnectToLocalWidget.h"
#include "SessionSetup/OrbitServiceInstance.h"

namespace orbit_session_setup {

class MockOrbitServiceInstance : public OrbitServiceInstance {
 public:
  MOCK_METHOD(bool, IsRunning, (), (override, const));
  MOCK_METHOD(ErrorMessageOr<void>, Shutdown, (), (override));
};

class ConnectToLocalWidgetTest : public ::testing::Test {
 public:
  void SetUp() override {
    start_orbit_service_button_ = widget_.findChild<QPushButton*>("startOrbitServiceButton");
    ASSERT_NE(start_orbit_service_button_, nullptr);
    status_label_ = widget_.findChild<QLabel*>("statusLabel");
    ASSERT_NE(status_label_, nullptr);
    content_frame_ = widget_.findChild<QFrame*>("contentFrame");
    ASSERT_NE(content_frame_, nullptr);
    radio_button_ = widget_.findChild<QRadioButton*>("radioButton");
    ASSERT_NE(radio_button_, nullptr);

    EXPECT_FALSE(start_orbit_service_button_->isEnabled());
  }

 protected:
  ConnectToLocalWidget widget_;
  QPushButton* start_orbit_service_button_;
  QLabel* status_label_;
  QFrame* content_frame_;
  QRadioButton* radio_button_;
};

TEST_F(ConnectToLocalWidgetTest, RadioButton) {
  // Default: radio_button_ not checked, content_frame_ not enabled
  EXPECT_TRUE(radio_button_->isEnabled());
  EXPECT_FALSE(content_frame_->isEnabled());
  EXPECT_FALSE(radio_button_->isChecked());

  QTest::mouseClick(radio_button_, Qt::LeftButton);
  EXPECT_TRUE(radio_button_->isEnabled());
  EXPECT_TRUE(content_frame_->isEnabled());
  EXPECT_TRUE(radio_button_->isChecked());
}

TEST_F(ConnectToLocalWidgetTest, OrbitServiceStartedSuccessfullyThenStopped) {
  bool lambda_called = false;
  widget_.SetOrbitServiceInstanceCreateFunction(
      [&lambda_called]() -> ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> {
        lambda_called = true;
        return std::make_unique<MockOrbitServiceInstance>();
      });

  // Enable the UI
  QTest::mouseClick(radio_button_, Qt::LeftButton);

  EXPECT_TRUE(start_orbit_service_button_->isEnabled());

  constexpr int kWaitTime = 500;  // Double of the timer interval

  QTest::qWait(kWaitTime);
  EXPECT_EQ(status_label_->text(), "Waiting for OrbitService");

  QTest::mouseClick(start_orbit_service_button_, Qt::LeftButton);

  EXPECT_TRUE(lambda_called);

  QTest::qWait(kWaitTime);
  EXPECT_EQ(status_label_->text(), "Connecting to OrbitService ...");
}

TEST_F(ConnectToLocalWidgetTest, OrbitServiceStartError) {
  bool lambda_called = false;
  widget_.SetOrbitServiceInstanceCreateFunction(
      [&lambda_called]() -> ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> {
        lambda_called = true;
        return ErrorMessage{"error"};
      });

  // Enable the UI
  QTest::mouseClick(radio_button_, Qt::LeftButton);

  EXPECT_TRUE(start_orbit_service_button_->isEnabled());

  QMetaObject::invokeMethod(
      &widget_,
      [&]() {
        auto* message_box = widget_.findChild<QMessageBox*>();
        ASSERT_NE(message_box, nullptr);
        message_box->close();
      },
      Qt::QueuedConnection);

  QTest::mouseClick(start_orbit_service_button_, Qt::LeftButton);

  EXPECT_TRUE(lambda_called);
}

}  // namespace orbit_session_setup