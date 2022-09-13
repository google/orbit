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
#include "OrbitBase/NotFoundOr.h"
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
using orbit_base::NotFoundOr;
using orbit_ggp::SymbolDownloadInfo;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;
using SymbolDownloadQuery = orbit_ggp::Client::SymbolDownloadQuery;
using testing::ElementsAre;
using testing::FieldsAre;

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

  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances, std::nullopt);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Instance>>& instances) {
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingAllReserved) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances,
                                                  std::nullopt);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Instance>>& instances) {
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingWithProject) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Project project{"display name", "project/test/id"};
  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances, project);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Instance>>& instances) {
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingAllReservedWithProject) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Project project{"display name", "project/test/id"};
  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kAllReservedInstances, project);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Instance>>& instances) {
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  auto future =
      client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances, std::nullopt);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Instance>>& instances) {
    EXPECT_THAT(instances, HasError("OrbitMockGgpWorking instance list -s"));
    EXPECT_THAT(instances, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncClientGetsDestroyed) {
  Future<ErrorMessageOr<QVector<Instance>>> future =
      Future<ErrorMessageOr<QVector<Instance>>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetInstancesAsync(Client::InstanceListScope::kOnlyOwnInstances,
                                               std::nullopt);
    future.Then(main_thread_executor_.get(),
                [](const ErrorMessageOr<QVector<Instance>>& instances_result) {
                  EXPECT_THAT(instances_result, HasError("orbit_ggp::Client no longer exists"));
                  QCoreApplication::exit();
                });
  }

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  QString test_instance_id = "instance/test/id";
  auto future = client.value()->GetSshInfoAsync(test_instance_id, std::nullopt);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_THAT(ssh_info, HasValue());
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncWorkingWithProject) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  QString test_instance_id = "instance/test/id";
  Project project{"display name", "project/test/id"};
  auto future = client.value()->GetSshInfoAsync(test_instance_id, project);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_THAT(ssh_info, HasValue());
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  QString test_instance_id = "instance/test/id";
  auto future = client.value()->GetSshInfoAsync(test_instance_id, std::nullopt);
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_THAT(ssh_info, HasError("OrbitMockGgpWorking ssh init -s --instance instance/test/id"));
    EXPECT_THAT(ssh_info, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncClientGetsDestroyed) {
  Future<ErrorMessageOr<SshInfo>> future =
      Future<ErrorMessageOr<SshInfo>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    QString test_instance_id = "instance/test/id";
    future = client.value()->GetSshInfoAsync(test_instance_id, std::nullopt);
    future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<SshInfo>& ssh_info_result) {
      EXPECT_THAT(ssh_info_result, HasError("orbit_ggp::Client no longer exists"));
      QCoreApplication::exit();
    });
  }

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetProjectsAsync();
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Project>>& project) {
    ASSERT_THAT(project, HasValue());
    EXPECT_EQ(project.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetProjectsAsync();
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Project>>& projects) {
    EXPECT_THAT(projects, HasError("OrbitMockGgpWorking project list -s"));
    EXPECT_THAT(projects, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncClientGetsDestroyed) {
  Future<ErrorMessageOr<QVector<Project>>> future =
      Future<ErrorMessageOr<QVector<Project>>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetProjectsAsync();
    future.Then(
        main_thread_executor_.get(), [](const ErrorMessageOr<QVector<Project>>& projects_result) {
          EXPECT_THAT(projects_result, HasError("killed because the parent object was destroyed"));
          QCoreApplication::exit();
        });
  }

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetDefaultProjectAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetDefaultProjectAsync();
  future.Then(main_thread_executor_.get(), [](ErrorMessageOr<Project> project) {
    EXPECT_THAT(project, HasValue(FieldsAre("Test Project", "Test Project id")));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetDefaultProjectAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetDefaultProjectAsync();
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<Project>& project) {
    EXPECT_THAT(project, HasError("OrbitMockGgpWorking config describe -s"));
    EXPECT_THAT(project, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetDefaultProjectAsyncClientGetsDestroyed) {
  Future<ErrorMessageOr<Project>> future =
      Future<ErrorMessageOr<Project>>{ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetDefaultProjectAsync();
    future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<Project>& project) {
      EXPECT_THAT(project, HasError("killed because the parent object was destroyed"));
      QCoreApplication::exit();
    });
  }

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, DescribeInstanceAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->DescribeInstanceAsync("id/of/instance1");
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<Instance>& instance) {
    ASSERT_THAT(instance, HasValue());
    EXPECT_EQ("id/of/instance1", instance.value().id);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, DescribeInstanceAsyncWorkingForInvalidInstance) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->DescribeInstanceAsync("unknown/instance");
  future.Then(main_thread_executor_.get(), [](const ErrorMessageOr<Instance>& instance) {
    ASSERT_THAT(instance, HasError("Unable to parse JSON"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetAccountAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  client.value()->GetDefaultAccountAsync().Then(
      main_thread_executor_.get(), [](ErrorMessageOr<Account> account) {
        ASSERT_THAT(account, HasValue());
        EXPECT_EQ(account.value().email, "username@email.com");
        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSymbolDownloadInfosAsyncWorking) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetSymbolDownloadInfoAsync({"symbol_filename_0", "build_id_0"});
  future.Then(main_thread_executor_.get(),
              [](const ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>& query_result) {
                EXPECT_THAT(query_result, HasNoError());
                EXPECT_FALSE(orbit_base::IsNotFound(query_result.value()));
                const auto symbol_download_info = orbit_base::GetFound(query_result.value());
                EXPECT_EQ(symbol_download_info.file_id, "symbolFiles/build_id_0/symbol_filename_0");
                EXPECT_EQ(symbol_download_info.url, "valid_url_for_symbol_0");
                QCoreApplication::exit();
              });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSymbolDownloadInfosAsyncNotFound) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetSymbolDownloadInfoAsync({"not_found_filename", "build_id_0"});
  future.Then(main_thread_executor_.get(),
              [](const ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>& query_result) {
                EXPECT_THAT(query_result, HasNoError());
                EXPECT_TRUE(orbit_base::IsNotFound(query_result.value()));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSymbolDownloadInfosAsyncError) {
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  auto future =
      client.value()->GetSymbolDownloadInfoAsync({"not_found_filename", "invalid_build_id"});
  future.Then(main_thread_executor_.get(),
              [](const ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>& query_result) {
                EXPECT_THAT(query_result,
                            HasError("incorrectly formatted build ID \"invalid_build_id\": "
                                     "encoding/hex: odd length hex string"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSymbolDownloadInfoAsyncTimeout) {
  // mock_ggp_working_ has a 50ms sleep, hence waiting for only 5ms should result in a timeout
  auto client = CreateClient(QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  auto future = client.value()->GetSymbolDownloadInfoAsync({"symbol_filename_0", "build_id_0"});
  future.Then(main_thread_executor_.get(),
              [](const ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>& query_result) {
                EXPECT_THAT(query_result,
                            HasError("OrbitMockGgpWorking crash-report download-symbols -s "
                                     "--show-url --module build_id_0/symbol_filename_0"));
                EXPECT_THAT(query_result, HasError("timed out after 5ms"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();
}

TEST_F(OrbitGgpClientTest, GetSymbolDownloadInfoAsyncClientGetsDestroyed) {
  Future<ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>> future = {
      ErrorMessage{"Empty Error Message"}};
  {
    ErrorMessageOr<std::unique_ptr<Client>> client =
        CreateClient(QString::fromStdString(mock_ggp_working_.string()));
    ASSERT_THAT(client, HasValue());

    future = client.value()->GetSymbolDownloadInfoAsync({"symbol_filename_0", "build_id_0"});
    future.Then(
        main_thread_executor_.get(),
        [](const ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>& query_result) {
          EXPECT_THAT(query_result, HasError("OrbitMockGgpWorking crash-report download-symbols -s "
                                             "--show-url --module build_id_0/symbol_filename_0"));
          EXPECT_THAT(query_result, HasError("killed because the parent object was destroyed"));
          QCoreApplication::exit();
        });
  }

  QCoreApplication::exec();
}

}  // namespace orbit_ggp