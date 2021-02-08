// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <QMetaObject>
#include <Qt>
#include <memory>
#include <outcome.hpp>
#include <system_error>

#include "EventLoop.h"
#include "gtest/gtest.h"

TEST(EventLoop, exec) {
  // Case 1: The event loop finishes successfully
  {
    orbit_qt::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());

    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.quit();
        },
        Qt::QueuedConnection);
    {
      const auto result = loop.exec();
      ASSERT_FALSE(result.has_error());
    }
  }

  // Case 2: The event loop returns an error that occured
  // while processing events/tasks.
  {
    orbit_qt::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());

    QMetaObject::invokeMethod(
        &loop,
        [&]() {
          ASSERT_TRUE(loop.isRunning());
          loop.error(std::make_error_code(std::errc::bad_message));
        },
        Qt::QueuedConnection);
    {
      const auto result = loop.exec();
      ASSERT_TRUE(result.has_error());
    }
  }

  // Case 3: The event loop immediately returns due to a queued error.
  {
    orbit_qt::EventLoop loop{};
    ASSERT_FALSE(loop.isRunning());
    loop.error(std::make_error_code(std::errc::bad_message));

    QMetaObject::invokeMethod(
        &loop,
        []() {
          FAIL();  // This task will be queued but never executes since the event loop is supposed
                   // to return early.
        },
        Qt::QueuedConnection);

    {
      const auto result = loop.exec();
      ASSERT_TRUE(result.has_error());
      ASSERT_EQ(result.error(), std::errc::bad_message);
    }
  }
}