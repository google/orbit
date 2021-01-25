// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <memory>

#include "MainThreadExecutorImpl.h"

namespace orbit_qt {

static int argc = 0;

TEST(MainThreadExecutorImpl, Schedule) {
  QCoreApplication app{argc, nullptr};

  auto executor = MainThreadExecutorImpl::Create();
  (void)executor->Schedule([]() { QCoreApplication::exit(42); });

  EXPECT_EQ(app.exec(), 42);
}

TEST(MainThreadExecutorImpl, Wait) {
  QCoreApplication app{argc, nullptr};

  bool called = false;
  auto executor = MainThreadExecutorImpl::Create();
  orbit_base::Future<void> future = executor->Schedule([&called]() { called = true; });
  EXPECT_EQ(executor->WaitFor(future), MainThreadExecutor::WaitResult::kCompleted);
}

}  // namespace orbit_qt