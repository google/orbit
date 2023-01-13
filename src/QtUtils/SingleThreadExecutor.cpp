// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/SingleThreadExecutor.h"

#include <QMetaObject>
#include <Qt>
#include <type_traits>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/Action.h"

namespace orbit_qt_utils {

SingleThreadExecutor::SingleThreadExecutor(QObject* parent) : QObject{parent} {
  thread_.start();
  context_.moveToThread(&thread_);
}

SingleThreadExecutor::~SingleThreadExecutor() {
  thread_.quit();
  thread_.wait();
}

void SingleThreadExecutor::ScheduleImpl(std::unique_ptr<Action> action) {
  QMetaObject::invokeMethod(
      &context_,
      [action = std::move(action)]() {
        ORBIT_SCOPE("SingleThreadExecutor Action");
        action->Execute();
      },
      Qt::QueuedConnection);
}

}  // namespace orbit_qt_utils