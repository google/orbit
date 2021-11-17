// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <functional>
#include <memory>
#include <optional>

#include "MainThreadExecutor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Project.h"
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
using testing::Return;

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
              (const QString& /*instance_id*/, std::optional<Project> /*project*/), (override));
  MOCK_METHOD(Future<ErrorMessageOr<QVector<Project>>>, GetProjectsAsync, (), (override));
  MOCK_METHOD(Future<ErrorMessageOr<Project>>, GetDefaultProjectAsync, (), (override));
  MOCK_METHOD(Future<ErrorMessageOr<Instance>>, DescribeInstanceAsync,
              (const QString& /*instance_id*/), (override));
  MOCK_METHOD(Future<ErrorMessageOr<orbit_ggp::Account>>, GetDefaultAccountAsync, (), (override));
};

namespace {

constexpr const char* kErrorString{"error"};

class RetrieveInstancesTest : public testing::Test {
 public:
  RetrieveInstancesTest()
      : executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()),
        retrieve_instances_(RetrieveInstances::Create(&mock_ggp_, executor_.get())) {}

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
      QCoreApplication::exit();
    });
    QCoreApplication::exec();
    EXPECT_TRUE(lambda_called);
  }

  template <typename T>
  static Future<ErrorMessageOr<T>> ReturnDefaultSuccessFuture() {
    return Future<ErrorMessageOr<T>>{T{}};
  }
  template <typename T>
  void VerifySuccessResult(Future<ErrorMessageOr<T>> future, std::function<void(T)> verifyer) {
    bool lambda_called = false;
    future.Then(executor_.get(),
                [&lambda_called, verifyer = std::move(verifyer)](ErrorMessageOr<T> result) {
                  EXPECT_FALSE(lambda_called);
                  lambda_called = true;
                  ASSERT_THAT(result, HasValue());
                  verifyer(result.value());
                  QCoreApplication::exit();
                });
    QCoreApplication::exec();
    EXPECT_TRUE(lambda_called);
  }
  template <typename T>
  void VerifyEqualSuccessResult(Future<ErrorMessageOr<T>> future, T value) {
    VerifySuccessResult<T>(std::move(future),
                           [value = std::move(value)](T result) { EXPECT_EQ(value, result); });
  }
  template <typename T>
  void VerifyDefaultSuccessResult(Future<ErrorMessageOr<T>> future) {
    VerifyEqualSuccessResult<T>(std::move(future), T{});
  }

 protected:
  MockGgpClient mock_ggp_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor_;
  std::unique_ptr<RetrieveInstances> retrieve_instances_;
};

}  // namespace

TEST_F(RetrieveInstancesTest, LoadInstancesCacheIsNotUsedWithError) {
  EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                           std::optional<Project>(std::nullopt)))
      .Times(3)
      .WillRepeatedly(&ReturnErrorFuture<QVector<Instance>>);

  VerifyErrorResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyErrorResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyErrorResult(retrieve_instances_->LoadInstances(
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
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));

  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kAllReservedInstances));

  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project, Client::InstanceListScope::kOnlyOwnInstances));

  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project, Client::InstanceListScope::kAllReservedInstances));

  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project_2, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project_2, Client::InstanceListScope::kOnlyOwnInstances));

  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project_2, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project_2, Client::InstanceListScope::kAllReservedInstances));

  // one more time each call
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      std::nullopt, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project, Client::InstanceListScope::kAllReservedInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project_2, Client::InstanceListScope::kOnlyOwnInstances));
  VerifyDefaultSuccessResult(retrieve_instances_->LoadInstances(
      test_project_2, Client::InstanceListScope::kAllReservedInstances));
}

TEST_F(RetrieveInstancesTest, LoadInstancesWithoutCacheAlwaysCallsGgpErrorCase) {
  const std::optional<Project> project = std::nullopt;

  {  // With error
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(project)))
        .Times(2)
        .WillRepeatedly(&ReturnErrorFuture<QVector<Instance>>);

    VerifyErrorResult(retrieve_instances_->LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
    VerifyErrorResult(retrieve_instances_->LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
  }

  {  // With success
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(project)))
        .Times(2)
        .WillRepeatedly(&ReturnDefaultSuccessFuture<QVector<Instance>>);

    VerifyDefaultSuccessResult(retrieve_instances_->LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
    VerifyDefaultSuccessResult(retrieve_instances_->LoadInstancesWithoutCache(
        project, Client::InstanceListScope::kOnlyOwnInstances));
  }
}

TEST_F(RetrieveInstancesTest, LoadProjectsAndInstancesError) {
  {  // all return error
    EXPECT_CALL(mock_ggp_, GetProjectsAsync()).WillOnce(&ReturnErrorFuture<QVector<Project>>);
    EXPECT_CALL(mock_ggp_, GetDefaultProjectAsync()).WillOnce(&ReturnErrorFuture<Project>);
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(std::nullopt)))
        .WillOnce(&ReturnErrorFuture<QVector<Instance>>);
    auto future = retrieve_instances_->LoadProjectsAndInstances(
        std::nullopt, Client::InstanceListScope::kOnlyOwnInstances);
    VerifyErrorResult(future);
  }

  {  // one returns error
    EXPECT_CALL(mock_ggp_, GetProjectsAsync()).WillOnce(&ReturnErrorFuture<QVector<Project>>);

    EXPECT_CALL(mock_ggp_, GetDefaultProjectAsync()).WillOnce(&ReturnDefaultSuccessFuture<Project>);
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(std::nullopt)))
        .WillOnce(&ReturnDefaultSuccessFuture<QVector<Instance>>);
    auto future = retrieve_instances_->LoadProjectsAndInstances(
        std::nullopt, Client::InstanceListScope::kOnlyOwnInstances);
    VerifyErrorResult(future);
  }

  {
    // if project is already nullopt and the call fails, there will be not be a retry (only one call
    // to GetInstancesAsync).
    EXPECT_CALL(mock_ggp_, GetProjectsAsync())
        .WillOnce(&ReturnDefaultSuccessFuture<QVector<Project>>);
    EXPECT_CALL(mock_ggp_, GetDefaultProjectAsync()).WillOnce(&ReturnDefaultSuccessFuture<Project>);
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(std::nullopt)))
        .WillOnce(&ReturnErrorFuture<QVector<Instance>>);

    auto future = retrieve_instances_->LoadProjectsAndInstances(
        std::nullopt, Client::InstanceListScope::kOnlyOwnInstances);
    VerifyErrorResult(future);
  }

  {
    // If project is not nullopt and the call has an error that contains "it may not exist", there
    // will be a retry (second call to GetInstancesAsync with nullopt). If the second try still has
    // an error, the final result will be an error.
    const Project test_project{"Test Display Name", "test_project_id"};

    EXPECT_CALL(mock_ggp_, GetProjectsAsync())
        .WillOnce(&ReturnDefaultSuccessFuture<QVector<Project>>);
    EXPECT_CALL(mock_ggp_, GetDefaultProjectAsync()).WillOnce(&ReturnDefaultSuccessFuture<Project>);
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(test_project)))
        .WillOnce(Return(ErrorMessage("it may not exist")));
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(std::nullopt)))
        .WillOnce(&ReturnErrorFuture<QVector<Instance>>);

    auto future = retrieve_instances_->LoadProjectsAndInstances(
        test_project, Client::InstanceListScope::kOnlyOwnInstances);
    VerifyErrorResult(future);
  }
}

TEST_F(RetrieveInstancesTest, LoadProjectsAndInstancesSuccess) {
  Project default_project{"Test Project 1 - default", "proj_id_1"};
  Project project_of_instances{"Test Project 2 - instances project", "proj_id_2"};
  Project test_project{"Project Display Name", "project_id"};
  QVector<Project> projects = {default_project, project_of_instances, test_project};

  Instance test_instance_of_default_1{"Test Instance Default 1", "instance_default_id_1"};
  Instance test_instance_of_default_2{"Test Instance Default 2", "instance_default_id_2"};
  QVector<Instance> instances_of_default_project = {test_instance_of_default_1,
                                                    test_instance_of_default_2};

  Instance test_instance_of_project_1{"Test Instance Project 1", "instance_project_id_1"};
  Instance test_instance_of_project_2{"Test Instance Project 2", "instance_project_id_2"};
  QVector<Instance> instances_of_project = {test_instance_of_project_1, test_instance_of_project_2};

  {  // all succeed
    EXPECT_CALL(mock_ggp_, GetProjectsAsync())
        .WillOnce(Return(Future<ErrorMessageOr<QVector<Project>>>(projects)));
    EXPECT_CALL(mock_ggp_, GetDefaultProjectAsync())
        .WillOnce(Return(Future<ErrorMessageOr<Project>>(default_project)));
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(project_of_instances)))
        .WillOnce(Return(Future<ErrorMessageOr<QVector<Instance>>>(instances_of_project)));
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(std::nullopt)))
        .WillOnce(Return(Future<ErrorMessageOr<QVector<Instance>>>(instances_of_default_project)));

    auto future = retrieve_instances_->LoadProjectsAndInstances(
        project_of_instances, Client::InstanceListScope::kOnlyOwnInstances);

    VerifySuccessResult<RetrieveInstances::LoadProjectsAndInstancesResult>(
        future,
        [default_project, project_of_instances, projects, instances_of_project](auto result) {
          EXPECT_EQ(result.default_project, default_project);
          EXPECT_EQ(result.project_of_instances, project_of_instances);
          EXPECT_EQ(result.projects, projects);
          EXPECT_EQ(result.instances, instances_of_project);
        });
  }

  {
    // If project is not nullopt and the call has and error containing "it may not exist", the
    // result of the call with the default will be used (second call to GetInstancesAsync with
    // nullopt). If that one succeeds, the whole call is successful.
    EXPECT_CALL(mock_ggp_, GetProjectsAsync())
        .WillOnce(Return(Future<ErrorMessageOr<QVector<Project>>>(projects)));
    EXPECT_CALL(mock_ggp_, GetDefaultProjectAsync())
        .WillOnce(Return(Future<ErrorMessageOr<Project>>(default_project)));
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(project_of_instances)))
        .WillOnce(Return(ErrorMessage("it may not exist")));
    EXPECT_CALL(mock_ggp_, GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                             std::optional<Project>(std::nullopt)))
        .WillOnce(Return(Future<ErrorMessageOr<QVector<Instance>>>(instances_of_default_project)));

    auto future = retrieve_instances_->LoadProjectsAndInstances(
        project_of_instances, Client::InstanceListScope::kOnlyOwnInstances);
    VerifySuccessResult<RetrieveInstances::LoadProjectsAndInstancesResult>(
        future, [default_project, projects, instances_of_default_project](auto result) {
          EXPECT_EQ(result.default_project, default_project);
          EXPECT_EQ(result.project_of_instances, std::nullopt);
          EXPECT_EQ(result.projects, projects);
          EXPECT_EQ(result.instances, instances_of_default_project);
        });
  }
}

}  // namespace orbit_session_setup