// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <chrono>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/SshInfo.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(OrbitGgpClient, CreateFailing) {
  {
    constexpr const char* kNonExistentProgram = "path/to/not/existing/program";
    auto client = Client::Create(nullptr, kNonExistentProgram);
    // The error messages on Windows and Linux are different, the only common part is the word
    // "file".
    EXPECT_THAT(client, HasError("file"));
  }
  {
    const std::filesystem::path mock_ggp_failing =
        orbit_base::GetExecutableDir() / "OrbitMockGgpFailing";
    auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_failing.string()));
    EXPECT_THAT(client, HasError("exit code: 5"));
  }
}

TEST(OrbitGgpClient, GetInstancesAsyncWorking) {
  const std::filesystem::path mock_ggp_working =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working.string()));
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  client.value()->GetInstancesAsync(
      [&callback_was_called](ErrorMessageOr<QVector<Instance>> instances) {
        QCoreApplication::exit();
        callback_was_called = true;
        ASSERT_TRUE(instances.has_value());
        EXPECT_EQ(instances.value().size(), 2);
      });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST(OrbitGgpClient, GetInstancesAsyncWorkingAllReserved) {
  const std::filesystem::path mock_ggp_working =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working.string()));
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  client.value()->GetInstancesAsync(
      [&callback_was_called](ErrorMessageOr<QVector<Instance>> instances) {
        QCoreApplication::exit();
        callback_was_called = true;
        ASSERT_TRUE(instances.has_value());
        EXPECT_EQ(instances.value().size(), 2);
      },
      true);

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST(OrbitGgpClient, GetInstancesAsyncTimeout) {
  const std::filesystem::path mock_ggp_working_slow =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working_slow.string()),
                               std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  client.value()->GetInstancesAsync(
      [&callback_was_called](const ErrorMessageOr<QVector<Instance>>& instances) {
        QCoreApplication::exit();
        callback_was_called = true;
        EXPECT_THAT(instances, HasError("Process request timed out after 5ms"));
      },
      0);

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST(OrbitGgpClient, GetSshInfoAsyncWorking) {
  const std::filesystem::path mock_ggp_working =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working.string()));
  ASSERT_THAT(client, HasValue());

  Instance test_instance;
  test_instance.id = "instance/test/id";

  bool callback_was_called = false;
  client.value()->GetSshInfoAsync(test_instance,
                                  [&callback_was_called](const ErrorMessageOr<SshInfo>& ssh_info) {
                                    QCoreApplication::exit();
                                    callback_was_called = true;
                                    EXPECT_TRUE(ssh_info.has_value());
                                  });
  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST(OrbitGgpClient, GetSshInfoAsyncTimeout) {
  const std::filesystem::path mock_ggp_working_slow =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  Instance test_instance;
  test_instance.id = "instance/test/id";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working_slow.string()),
                               std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  client.value()->GetSshInfoAsync(
      test_instance, [&callback_was_called](const ErrorMessageOr<SshInfo>& ssh_info) {
        QCoreApplication::exit();
        callback_was_called = true;
        EXPECT_THAT(ssh_info, HasError("Process request timed out after 5ms"));
      });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST(OrbitGgpClient, GetProjectsAsyncWorking) {
  const std::filesystem::path mock_ggp_working =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working.string()));
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  client.value()->GetProjectsAsync(
      [&callback_was_called](ErrorMessageOr<QVector<Project>> project) {
        QCoreApplication::exit();
        callback_was_called = true;
        ASSERT_TRUE(project.has_value());
        EXPECT_EQ(project.value().size(), 2);
      });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

TEST(OrbitGgpClient, GetProjectsAsyncTimeout) {
  const std::filesystem::path mock_ggp_working_slow =
      orbit_base::GetExecutableDir() / "OrbitMockGgpWorking";

  auto client = Client::Create(nullptr, QString::fromStdString(mock_ggp_working_slow.string()),
                               std::chrono::milliseconds{5});
  ASSERT_THAT(client, HasValue());

  bool callback_was_called = false;
  client.value()->GetProjectsAsync(
      [&callback_was_called](const ErrorMessageOr<QVector<Project>>& projects) {
        QCoreApplication::exit();
        callback_was_called = true;
        EXPECT_THAT(projects, HasError("Process request timed out after 5ms"));
      });

  QCoreApplication::exec();

  EXPECT_TRUE(callback_was_called);
}

}  // namespace orbit_ggp