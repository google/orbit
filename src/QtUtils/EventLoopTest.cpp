// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QMetaObject>
#include <Qt>
#include <string>
#include <system_error>

#include "OrbitBase/Result.h"
#include "QtUtils/EventLoop.h"
#include "TestUtils/TestUtils.h"

TEST(EventLoop, exec) {
  // Case 1: The event loop finishes successfully
  {
    orbit_qt_utils::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());

    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.quit();
        },
        Qt::QueuedConnection);
    EXPECT_THAT(loop.exec(), orbit_test_utils::HasValue(0));
  }

  // Case 2: The event loop returns an error that occured
  // while processing events/tasks.
  {
    orbit_qt_utils::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());

    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.error(std::make_error_code(std::errc::bad_message));
        },
        Qt::QueuedConnection);
    EXPECT_THAT(loop.exec(),
                orbit_test_utils::HasError(
                    ErrorMessage{std::make_error_code(std::errc::bad_message)}.message()));
  }

  // Case 3: The event loop immediately returns due to a queued error.
  {
    orbit_qt_utils::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());
    loop.error(std::make_error_code(std::errc::bad_message));

    QMetaObject::invokeMethod(
        &loop,
        []() {
          FAIL();  // This task will be queued but never executes since the event loop is supposed
                   // to return early.
        },
        Qt::QueuedConnection);
    EXPECT_THAT(loop.exec(),
                orbit_test_utils::HasError(
                    ErrorMessage{std::make_error_code(std::errc::bad_message)}.message()));
  }

  // Case 4: The event loop immediately returns due to a queued result (quit).
  {
    orbit_qt_utils::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());
    loop.quit();

    QMetaObject::invokeMethod(
        &loop,
        []() {
          FAIL();  // This task will be queued but never executes since the event loop is supposed
                   // to return early.
        },
        Qt::QueuedConnection);

    EXPECT_THAT(loop.exec(), orbit_test_utils::HasValue(0));
  }
}

TEST(EventLoop, exit) {
  orbit_qt_utils::EventLoop loop{};
  ASSERT_FALSE(loop.isRunning());

  QMetaObject::invokeMethod(
      &loop,
      [&]() {
        ASSERT_TRUE(loop.isRunning());
        loop.exit(42);
      },
      Qt::QueuedConnection);
  EXPECT_THAT(loop.exec(), orbit_test_utils::HasValue(42));
}

TEST(EventLoop, processEvents) {
  orbit_qt_utils::EventLoop loop{};
  ASSERT_FALSE(loop.isRunning());

  bool called = false;
  QMetaObject::invokeMethod(
      &loop, [&]() { called = true; }, Qt::QueuedConnection);

  loop.processEvents();
  EXPECT_TRUE(called);
}

TEST(EventLoop, reuseLoop) {
  // Testing whether Eventloop can be reused, similar to QEventloop

  orbit_qt_utils::EventLoop loop{};
  ASSERT_FALSE(loop.isRunning());

  // 1. normal quit
  {
    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.quit();
        },
        Qt::QueuedConnection);
    EXPECT_THAT(loop.exec(), orbit_test_utils::HasValue(0));
  }

  // 2. normal error from error code
  {
    const auto error_code = std::make_error_code(std::errc::bad_message);
    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.error(error_code);
        },
        Qt::QueuedConnection);
    EXPECT_THAT(loop.exec(), orbit_test_utils::HasError(ErrorMessage{error_code}.message()));
  }

  // 3. normal error from ErrorMessage
  {
    const ErrorMessage error_message{"Important error message"};
    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.error(error_message);
        },
        Qt::QueuedConnection);

    EXPECT_THAT(loop.exec(), orbit_test_utils::HasError(error_message.message()));
  }

  // 4. premature quit
  {
    loop.quit();
    QMetaObject::invokeMethod(
        &loop,
        []() {
          FAIL();  // This task will be queued but never executes since the event loop is supposed
                   // to return early.
        },
        Qt::QueuedConnection);

    EXPECT_THAT(loop.exec(), orbit_test_utils::HasValue(0));
  }

  // 5. premature error from error code
  {
    const auto error_code = std::make_error_code(std::errc::bad_message);
    loop.error(error_code);
    EXPECT_THAT(loop.exec(), orbit_test_utils::HasError(ErrorMessage{error_code}.message()));
  }

  // 6. premature error from ErrorMessage
  {
    const ErrorMessage error_message{"Important error message"};
    loop.error(error_message);
    EXPECT_THAT(loop.exec(), orbit_test_utils::HasError(error_message.message()));
  }
}