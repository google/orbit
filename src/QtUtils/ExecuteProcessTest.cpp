// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <absl/time/time.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QByteArray>
#include <QObject>
#include <QTimer>
#include <memory>

#include "MainThreadExecutor.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "QtUtils/AssertNoQtLogWarnings.h"
#include "QtUtils/ExecuteProcess.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "TestUtils/TestUtils.h"

namespace orbit_qt_utils {

using orbit_base::Future;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(QtUtilsExecuteProcess, ProgramNotFound) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess("non_existing_process", QStringList{}, QApplication::instance());

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    EXPECT_THAT(result, HasError("Error occurred while executing process"));
    EXPECT_THAT(result, HasError("non_existing_process"));
    EXPECT_THAT(result, HasError("FailedToStart"));
    QApplication::exit();
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, ReturnsFailExitCode) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{"--exit_code", "240"}, QApplication::instance());

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    EXPECT_THAT(result, HasError("failed with exit code: 240"));
    QApplication::exit();
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, Succeeds) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{}, QApplication::instance());

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    ASSERT_THAT(result, HasValue());
    EXPECT_TRUE(absl::StrContains(result.value().toStdString(), "Some example output"));
    QApplication::exit();
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, SucceedsWithoutParent) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  // Note, this call omits the parent argument which then defaults to nullptr
  Future<ErrorMessageOr<QByteArray>> future = ExecuteProcess(program, QStringList{});

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    ASSERT_THAT(result, HasValue());
    EXPECT_TRUE(absl::StrContains(result.value().toStdString(), "Some example output"));
    QApplication::exit();
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, SucceedsWithSleep) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{"--sleep_for_ms", "200"}, QApplication::instance());

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    ASSERT_THAT(result, HasValue());
    EXPECT_TRUE(absl::StrContains(result.value().toStdString(), "Some example output"));
    EXPECT_TRUE(absl::StrContains(result.value().toStdString(), "Slept for 200ms"));
    QApplication::exit();
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, FailsBecauseOfTimeout) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{"--sleep_for_ms", "200"}, QApplication::instance(),
                     absl::Milliseconds(100));

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    EXPECT_THAT(result, HasError("timed out after 100ms"));

    // QApplication is not quit immediately here, to allow clean up (killing and deletion of the
    // process), which is queued in the event loop.
    QTimer::singleShot(5, QApplication::instance(), &QApplication::quit);
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, FailsBecauseOfTimeoutWithValueZero) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{"--sleep_for_ms", "200"}, QApplication::instance(),
                     absl::ZeroDuration());

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;
    EXPECT_THAT(result, HasError("timed out after 0ms"));

    // QApplication is not quit immediately here, to allow clean up (killing and deletion of the
    // process), which is queued in the event loop.
    QTimer::singleShot(5, QApplication::instance(), &QApplication::quit);
  });

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, ParentGetsDeletedImmediately) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  QObject* parent_object = new QObject{};

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{"--sleep_for_ms", "200"}, parent_object);

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;

    EXPECT_THAT(result, HasError("killed because the parent object was destroyed"));

    // QApplication is not quit immediately here, to allow clean up (killing and deletion of the
    // process), which is queued in the event loop.
    QTimer::singleShot(5, QApplication::instance(), &QApplication::quit);
  });
  parent_object->deleteLater();

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

TEST(QtUtilsExecuteProcess, ParentGetsDeletedWhileExecuting) {
  AssertNoQtLogWarnings message_handler{};
  std::shared_ptr<MainThreadExecutor> mte = MainThreadExecutorImpl::Create();

  QString program =
      QString::fromStdString((orbit_base::GetExecutableDir() / "FakeCliProgram").string());

  QObject* parent_object = new QObject{};

  Future<ErrorMessageOr<QByteArray>> future =
      ExecuteProcess(program, QStringList{"--sleep_for_ms", "200"}, parent_object);

  bool lambda_was_called = false;
  future.Then(mte.get(), [&lambda_was_called](const ErrorMessageOr<QByteArray>& result) {
    EXPECT_FALSE(lambda_was_called);
    lambda_was_called = true;

    EXPECT_THAT(result, HasError("killed because the parent object was destroyed"));

    // QApplication is not quit immediately here, to allow clean up (killing and deletion of the
    // process), which is queued in the event loop.
    QTimer::singleShot(5, QApplication::instance(), &QApplication::quit);
  });

  QTimer::singleShot(100, parent_object, &QObject::deleteLater);

  QApplication::exec();

  EXPECT_TRUE(lambda_was_called);
}

}  // namespace orbit_qt_utils