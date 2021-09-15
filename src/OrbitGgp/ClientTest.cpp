// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <chrono>
#include <memory>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/Project.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

namespace {

class OrbitGgpClientTest : public ::testing::Test {
 public:
  OrbitGgpClientTest() : mte_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {}

 protected:
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> mte_;
  const std::filesystem::path mock_ggp_working_ =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";
};

}  // namespace

TEST_F(OrbitGgpClientTest, CreateFailing) {
  {
    constexpr const char* kNonExistentProgram = "path/to/not/existing/program";
    auto client = CreateClient(mte_.get(), kNonExistentProgram);
    // The error messages on Windows and Linux are different, the only common part is the word
    // "file".
    EXPECT_THAT(client, HasError("file"));
  }
  {
    const std::filesystem::path mock_ggp_failing =
        orbit_base::GetExecutableDir() / "OrbitMockGgpFailing";
    auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_failing.string()));
    EXPECT_THAT(client, HasError("exit code: 5"));
  }
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorking) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;

  auto future = client.value()->GetInstancesAsync();
  future.Then(mte_.get(), [&callback_was_called](ErrorMessageOr<QVector<Instance>> instances) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingAllReserved) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  auto future = client.value()->GetInstancesAsync(true);
  future.Then(mte_.get(), [&callback_was_called](ErrorMessageOr<QVector<Instance>> instances) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingWithProject) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Project project{"display name", "project/test/id"};

  bool callback_was_called = false;

  auto future = client.value()->GetInstancesAsync(false, project);
  future.Then(mte_.get(), [&callback_was_called](ErrorMessageOr<QVector<Instance>> instances) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncWorkingAllReservedWithProject) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Project project{"display name", "project/test/id"};

  bool callback_was_called = false;

  auto future = client.value()->GetInstancesAsync(true, project);
  future.Then(mte_.get(), [&callback_was_called](ErrorMessageOr<QVector<Instance>> instances) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    ASSERT_THAT(instances, HasValue());
    EXPECT_EQ(instances.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetInstancesAsyncTimeout) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  auto future = client.value()->GetInstancesAsync();
  future.Then(mte_.get(),
              [&callback_was_called](const ErrorMessageOr<QVector<Instance>>& instances) {
                EXPECT_FALSE(callback_was_called);
                callback_was_called = true;
                EXPECT_THAT(instances, HasError("OrbitMockGgpWorking instance list -s"));
                EXPECT_THAT(instances, HasError("timed out after 5ms"));
                QCoreApplication::exit();
              });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncWorking) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Instance test_instance;
  test_instance.id = "instance/test/id";

  bool callback_was_called = false;
  auto future = client.value()->GetSshInfoAsync(test_instance);
  future.Then(mte_.get(), [&callback_was_called](const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    EXPECT_THAT(ssh_info, HasValue());
    QCoreApplication::exit();
  });
  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncWorkingWithProject) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  Instance test_instance;
  test_instance.id = "instance/test/id";

  Project project{"display name", "project/test/id"};

  bool callback_was_called = false;
  auto future = client.value()->GetSshInfoAsync(test_instance, project);
  future.Then(mte_.get(), [&callback_was_called](const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    EXPECT_THAT(ssh_info, HasValue());
    QCoreApplication::exit();
  });
  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetSshInfoAsyncTimeout) {
  Instance test_instance;
  test_instance.id = "instance/test/id";

  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  auto future = client.value()->GetSshInfoAsync(test_instance);
  future.Then(mte_.get(), [&callback_was_called](const ErrorMessageOr<SshInfo>& ssh_info) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    EXPECT_THAT(ssh_info, HasError("OrbitMockGgpWorking ssh init -s --instance instance/test/id"));
    EXPECT_THAT(ssh_info, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncWorking) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()));
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;

  auto future = client.value()->GetProjectsAsync();
  future.Then(mte_.get(), [&callback_was_called](ErrorMessageOr<QVector<Project>> project) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    ASSERT_THAT(project, HasValue());
    EXPECT_EQ(project.value().size(), 2);
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST_F(OrbitGgpClientTest, GetProjectsAsyncTimeout) {
  auto client = CreateClient(mte_.get(), QString::fromStdString(mock_ggp_working_.string()),
                             std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;

  auto future = client.value()->GetProjectsAsync();
  future.Then(mte_.get(), [&callback_was_called](const ErrorMessageOr<QVector<Project>>& projects) {
    EXPECT_FALSE(callback_was_called);
    callback_was_called = true;
    EXPECT_THAT(projects, HasError("OrbitMockGgpWorking project list -s"));
    EXPECT_THAT(projects, HasError("timed out after 5ms"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

}  // namespace orbit_ggp