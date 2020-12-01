// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MainThreadExecutorImpl.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <list>
#include <thread>

#include "OrbitBase/Tracing.h"

namespace {

class MainThreadExecutorImpl : public MainThreadExecutor {
 public:
  MainThreadExecutorImpl() = default;

  void Schedule(std::unique_ptr<Action> action) override;
};

void MainThreadExecutorImpl::Schedule(std::unique_ptr<Action> action) {
  QMetaObject::invokeMethod(QCoreApplication::instance(), [action = std::move(action)]() {
    ORBIT_SCOPE("MainThreadExecutor Action");
    action->Execute();
  });
}

}  // namespace

std::unique_ptr<MainThreadExecutor> CreateMainThreadExecutor() {
  return std::make_unique<MainThreadExecutorImpl>();
}
