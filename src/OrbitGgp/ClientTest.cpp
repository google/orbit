// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <chrono>
#include <memory>
#include <optional>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Account.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/Project.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

using orbit_base::Future;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

namespace {

class OrbitGgpClientTest : public testing::Test {
 public:
  OrbitGgpClientTest() : main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()){};

 protected:
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> main_thread_executor_;
  const std::filesystem::path mock_ggp_working_ =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";
};

}  // namespace

TEST_F(OrbitGgpClientTest, CreateFailing) {
  {
    constexpr const char* kNonExistentProgram = "path/to/not/existing/program";
    auto client = CreateClient(kNonExistentProgram);
    // The error messages on Windows and Linux are different, the only common part is the word
    // "file".
    EXPECT_THAT(client, HasError("file"));
  }
  {
    const std::filesystem::path mock_ggp_failing =
        orbit_base::GetExecutableDir() / "OrbitMockGgpFailing";
    auto client = CreateClient(QString::fromStdString(mock_ggp_failing.string()));
    EXPECT_THAT(client, HasError("exit code: 5"));
  }
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances, std::nullopt);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<QVector<Instance>> instances) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(instances, HasValue());
                EXPECT_EQ(instances.value().size(), 2);
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingAllReserved) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;
  auto future = client.value()->GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances,
                                                  std::nullopt);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<QVector<Instance>> instances) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(instances, HasValue());
                EXPECT_EQ(instances.value().size(), 2);
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingWithProject) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Project project{"display name", "project/test/id"};

  bool future_is_resolved = false;

  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances, project);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<QVector<Instance>> instances) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(instances, HasValue());
                EXPECT_EQ(instances.value().size(), 2);
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingAllReservedWithProject) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Project project{"display name", "project/test/id"};

  bool future_is_resolved = false;

  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances, project);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<QVector<Instance>> instances) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(instances, HasValue());
                EXPECT_EQ(instances.value().size(), 2);
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;
  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances, std::nullopt);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](const ErrorMessageOr<QVector<Instance>>& instances) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                EXPECT_THAT(instances, HasError("OrbitMockGgpWorking instance list -s"));
                EXPECT_THAT(instances, HasError("timed out after 5ms"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncClientGetsDestroyed) {
  bool future_is_resolved = false;

  Future<ErrorMessageOr<QVector<Instance>>> future =
      Future<ErrorMessageOr<QVector<Instance>>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                               std::nullopt);

    future.Then(main_thread_executor_.get(),
                [&future_is_resolved](const ErrorMessageOr<QVector<Instance>>& instances_result) {
                  EXPECT_FALSE(future_is_resolved);
                  future_is_resolved = true;
                  EXPECT_THAT(instances_result, HasError("orbit_ggp::Client no longer exists"));
                  QCoreApplication::exit();
                });
  }

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  QString test_instance_id = "instance/test/id";

  bool future_is_resolved = false;
  auto future = client.value()->GetSshInfoAsync(test_instance_id, std::nullopt);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](const ErrorMessageOr<SshInfo>& ssh_info) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                EXPECT_THAT(ssh_info, HasValue());
                QCoreApplication::exit();
              });
  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncWorkingWithProject) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  QString test_instance_id = "instance/test/id";

  Project project{"display name", "project/test/id"};

  bool future_is_resolved = false;
  auto future = client.value()->GetSshInfoAsync(test_instance_id, project);
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](const ErrorMessageOr<SshInfo>& ssh_info) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                EXPECT_THAT(ssh_info, HasValue());
                QCoreApplication::exit();
              });
  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncTimeout) {
  QString test_instance_id = "instance/test/id";

  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;
  auto future = client.value()->GetSshInfoAsync(test_instance_id, std::nullopt);
  future.Then(main_thread_executor_.get(), [&future_is_resolved](
                                               const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_FALSE(future_is_resolved);
    future_is_resolved = true;
    EXPECT_THAT(ssh_info, HasError("OrbitMockGgpWorking ssh init -s --instance instance/test/id"));
    EXPECT_THAT(ssh_info, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncClientGetsDestroyed) {
  bool future_is_resolved = false;

  Future<ErrorMessageOr<SshInfo>> future =
      Future<ErrorMessageOr<SshInfo>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    QString test_instance_id = "instance/test/id";

    future = client.value()->GetSshInfoAsync(test_instance_id, std::nullopt);

    future.Then(main_thread_executor_.get(), [&future_is_resolved](
                                                 const ErrorMessageOr<SshInfo>& ssh_info_result) {
      EXPECT_FALSE(future_is_resolved);
      future_is_resolved = true;
      EXPECT_THAT(ssh_info_result, HasError("killed because the parent object was destroyed"));
      QCoreApplication::exit();
    });
  }

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future = client.value()->GetProjectsAsync();
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<QVector<Project>> project) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(project, HasValue());
                EXPECT_EQ(project.value().size(), 2);
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future = client.value()->GetProjectsAsync();
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](const ErrorMessageOr<QVector<Project>>& projects) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                EXPECT_THAT(projects, HasError("OrbitMockGgpWorking project list -s"));
                EXPECT_THAT(projects, HasError("timed out after 5ms"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncClientGetsDestroyed) {
  bool future_is_resolved = false;

  Future<ErrorMessageOr<QVector<Project>>> future =
      Future<ErrorMessageOr<QVector<Project>>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetProjectsAsync();

    future.Then(main_thread_executor_.get(),
                [&future_is_resolved](const ErrorMessageOr<QVector<Project>>& projects_result) {
                  EXPECT_FALSE(future_is_resolved);
                  future_is_resolved = true;
                  EXPECT_THAT(projects_result,
                              HasError("killed because the parent object was destroyed"));
                  QCoreApplication::exit();
                });
  }

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetDefaultProjectAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future = client.value()->GetDefaultProjectAsync();
  future.Then(main_thread_executor_.get(), [&future_is_resolved](ErrorMessageOr<Project> project) {
    EXPECT_FALSE(future_is_resolved);
    future_is_resolved = true;
    ASSERT_THAT(project, HasValue());
    EXPECT_EQ(project.value().display_name, "Test Project");
    EXPECT_EQ(project.value().id, "Test Project id");
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetDefaultProjectAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future = client.value()->GetDefaultProjectAsync();
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](const ErrorMessageOr<Project>& project) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                EXPECT_THAT(project, HasError("OrbitMockGgpWorking config describe -s"));
                EXPECT_THAT(project, HasError("timed out after 5ms"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetDefaultProjectAsyncClientGetsDestroyed) {
  bool future_is_resolved = false;

  Future<ErrorMessageOr<Project>> future =
      Future<ErrorMessageOr<Project>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetDefaultProjectAsync();

    future.Then(main_thread_executor_.get(),
                [&future_is_resolved](const ErrorMessageOr<Project>& project) {
                  EXPECT_FALSE(future_is_resolved);
                  future_is_resolved = true;
                  EXPECT_THAT(project, HasError("killed because the parent object was destroyed"));
                  QCoreApplication::exit();
                });
  }

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, DescribeInstanceAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future = client.value()->DescribeInstanceAsync("id/of/instance1");
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<Instance> instance) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(instance, HasValue());
                EXPECT_EQ("id/of/instance1", instance.value().id);
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, DescribeInstanceAsyncWorkingForInvalidInstance) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  auto future = client.value()->DescribeInstanceAsync("unknown/instance");
  future.Then(main_thread_executor_.get(),
              [&future_is_resolved](ErrorMessageOr<Instance> instance) {
                EXPECT_FALSE(future_is_resolved);
                future_is_resolved = true;
                ASSERT_THAT(instance, HasError("Unable to parse JSON"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

TEST_F(OrbitGgpClientTest, GetAccountAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool future_is_resolved = false;

  client.value()->GetDefaultAccountAsync().Then(
      main_thread_executor_.get(), [&future_is_resolved](ErrorMessageOr<Account> account) {
        EXPECT_FALSE(future_is_resolved);
        future_is_resolved = true;
        ASSERT_THAT(account, HasValue());
        EXPECT_EQ(account.value().email, "username@email.com");
        QCoreApplication::exit();
      });

  QCoreApplication::exec();

  EXPECT_TRUE(future_is_resolved);
}

}  // namespace orbit_ggp