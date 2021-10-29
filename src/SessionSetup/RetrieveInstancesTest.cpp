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
using orbit_ggp::Client;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using orbit_ggp::SshInfo;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

class MockGgpClient : public orbit_ggp::Client {
 public:
  MOCK_METHOD(Future<ErrorMessageOr<QVector<Instance>>>, GetInstancesAsync,
              (Client::InstanceListScope /*scope*/, std::optional<Project> /*project*/),
              (override));
  MOCK_METHOD(Future<ErrorMessageOr<QVector<Instance>>>, GetInstancesAsync,
              (Client::InstanceListScope /*scope*/, std::optional<Project> /*project*/,
               int /*retry*/),
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
        retrieve_instances_(&mock_ggp_, executor_.get(), QCoreApplication::instance()) {}

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
  RetrieveInstances retrieve_instances_;
};

}  // namespace

TEST_F(RetrieveInstancesTest, LoadInstancesCacheIsNotUsedWithError) {
  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                           std::optional<Project>(std::nullopt)))
      .Times(3)
      .WillRepeatedly(&ReturnErrorFuture<QVector<Instance>>);

  VerifyErrorResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyErrorResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyErrorResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
}

TEST_F(RetrieveInstancesTest, LoadInstancesCacheWorks) {
  // ggp is only called once for every combination
  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                           std::optional<Project>(std::nullopt)))
      .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);

  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances,
                                           std::optional<Project>(std::nullopt)))
      .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);

  const Project test_project{"Test Display Name", "test_project_id"};
  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                           std::optional<Project>(test_project)))
      .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);

  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances,
                                           std::optional<Project>(test_project)))
      .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);

  const Project test_project_2{"Test Display Name 2", "test_project_id_2"};
  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                           std::optional<Project>(test_project_2)))
      .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);

  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances,
                                           std::optional<Project>(test_project_2)))
      .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);

  // 2 times each combination
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));

  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kAllReservedInstances));

  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project, Client::InstanceListScope::kOnlyOwnInstances));

  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project, Client::InstanceListScope::kAllReservedInstances));

  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project_2, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project_2, Client::InstanceListScope::kOnlyOwnInstances));

  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project_2, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project_2, Client::InstanceListScope::kAllReservedInstances));

  // one more time each call
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      std::nullopt, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project_2, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_.LoadInstances(
      test_project_2, Client::InstanceListScope::kAllReservedInstances));
}

TEST_F(RetrieveInstancesTest, LoadInstancesWithoutCacheAlwaysCallsGgpErrorCase) {
  const std::optional<Project> project = std::nullopt;

  {  // With error
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(project)))
        .Times(2)
        .WillRepeatedly(&ReturnErrorFuture<QVector<Instance>>);

    VerifyErrorResult(retrieve_instances_.LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
    VerifyErrorResult(retrieve_instances_.LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
  }

  {  // With success
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(project)))
        .Times(2)
        .WillRepeatedly(&ReturnDefaultSuccessFuture<QVector<Instance>>);

    VerifyDefaultSuccessResult(retrieve_instances_.LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
    VerifyDefaultSuccessResult(retrieve_instances_.LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
  }
}

}  // namespace orbit_session_setup