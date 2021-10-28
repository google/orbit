// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <optional>

#include "MainThreadExecutor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/RetrieveInstances.h"
#include "TestUtils/TestUtils.h"

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using orbit_ggp::SshInfo;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

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

constexpr const char* kErrorString{"error"};

class RetrieveInstancesTest : public testing::Test {
 public:
  RetrieveInstancesTest()
      : executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()),
        retreive_instances_(&mock_ggp_, executor_.get(), QCoreApplication::instance()) {}

  template <typename T>
  static Future<ErrorMessageOr<T>> ReturnErrorFuture() {
    return ErrorMessage{kErrorString};
  }
  template <typename T>
  void VerifyErrorResult(Future<ErrorMessageOr<T>> future) {
    bool lambda_called = false;
    future.Then(executor_.get(), [&lambda_called](ErrorMessageOr<T> result) {
      EXPECT_FALSE(lambda_called);
      lambda_called = true;
      EXPECT_THAT(result, HasError(kErrorString));
    });
    QCoreApplication::processEvents();
    EXPECT_TRUE(lambda_called);
  }

  template <typename T>
  static Future<ErrorMessageOr<T>> ReturnDefaultSuccessFuture() {
    return Future<ErrorMessageOr<T>>{T{}};
  }
  template <typename T>
  void VerifyDefaultSuccessResult(Future<ErrorMessageOr<T>> future) {
    bool lambda_called = false;
    future.Then(executor_.get(), [&lambda_called](ErrorMessageOr<T> result) {
      EXPECT_FALSE(lambda_called);
      lambda_called = true;
      ASSERT_THAT(result, HasValue());
      EXPECT_EQ(result.value(), T{});
    });
    QCoreApplication::processEvents();
    EXPECT_TRUE(lambda_called);
  }

 protected:
  MockGgpClient mock_ggp_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor_;
  RetrieveInstances retreive_instances_;
};

}  // namespace

TEST_F(RetrieveInstancesTest, LoadInstancesCacheWorks) {
  const std::optional<Project> project = std::nullopt;

  {  // Error case: cache is not used
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(false, std::optional<Project>(project)))
        .Times(3)
        .WillRepeatedly(&ReturnErrorFuture<QVector<Instance>>);

    VerifyErrorResult(retreive_instances_.LoadInstances(project, false));
    VerifyErrorResult(retreive_instances_.LoadInstances(project, false));
    VerifyErrorResult(retreive_instances_.LoadInstances(project, false));
  }

  {  // Success case: cache is used (ggp is only called once)
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(true, std::optional<Project>(project)))
        .Times(1)
        .WillRepeatedly(&ReturnDefaultSuccessFuture<QVector<Instance>>);

    VerifyDefaultSuccessResult(retreive_instances_.LoadInstances(project, true));
    VerifyDefaultSuccessResult(retreive_instances_.LoadInstances(project, true));
    VerifyDefaultSuccessResult(retreive_instances_.LoadInstances(project, true));
  }
}

TEST_F(RetrieveInstancesTest, LoadInstancesWithoutCacheAlwaysCallsGgpErrorCase) {
  const std::optional<Project> project = std::nullopt;

  {  // With error
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(false, std::optional<Project>(project)))
        .Times(2)
        .WillRepeatedly(&ReturnErrorFuture<QVector<Instance>>);

    VerifyErrorResult(retreive_instances_.LoadInstancesWithoutCache(project, false));
    VerifyErrorResult(retreive_instances_.LoadInstancesWithoutCache(project, false));
  }

  {  // With success
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(false, std::optional<Project>(project)))
        .Times(2)
        .WillRepeatedly(&ReturnDefaultSuccessFuture<QVector<Instance>>);

    VerifyDefaultSuccessResult(retreive_instances_.LoadInstancesWithoutCache(project, false));
    VerifyDefaultSuccessResult(retreive_instances_.LoadInstancesWithoutCache(project, false));
  }
}

}  // namespace orbit_session_setup