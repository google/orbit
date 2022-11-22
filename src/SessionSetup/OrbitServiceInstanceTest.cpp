// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <memory>

#include "OrbitBase/Result.h"
#include "SessionSetup/OrbitServiceInstance.h"
#include "TestUtils/TestUtils.h"

namespace orbit_session_setup {

using orbit_test_utils::HasNoError;

TEST(OrbitServiceInstance, CreateAndShutdown) {
  const QString orbit_service_path = QCoreApplication::applicationDirPath() + "/OrbitService";
  ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> instance_or_error =
      OrbitServiceInstance::Create(orbit_service_path, {});

  ASSERT_THAT(instance_or_error, HasNoError());

  OrbitServiceInstance& instance = *instance_or_error.value();
  EXPECT_TRUE(instance.IsRunning());

  QSignalSpy spy{&instance, &OrbitServiceInstance::ErrorOccurred};

  EXPECT_THAT(instance.Shutdown(), HasNoError());

  EXPECT_EQ(spy.count(), 0);
}

TEST(OrbitServiceInstance, ProcessEndsUnexpectedly) {
  ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> instance_or_error =
      OrbitServiceInstance::Create("sleep", {"0.1"});  // sleep 100 ms
  ASSERT_THAT(instance_or_error, HasNoError());

  OrbitServiceInstance& instance = *instance_or_error.value();
  EXPECT_TRUE(instance.IsRunning());

  bool lambda_called = false;
  QObject::connect(&instance, &OrbitServiceInstance::ErrorOccurred, [&](const QString& message) {
    EXPECT_THAT(message.toStdString(), testing::HasSubstr("ended unexpectedly. exit code: 0"));
    lambda_called = true;
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
  EXPECT_TRUE(lambda_called);
}

}  // namespace orbit_session_setup
