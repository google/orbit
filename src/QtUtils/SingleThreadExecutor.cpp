// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/SingleThreadExecutor.h"

#include <QMetaObject>
#include <Qt>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/Action.h"

namespace orbit_qt_utils {

SingleThreadExecutor::SingleThreadExecutor(QObject* parent) : QObject{parent} {
  thread.start();
  context.moveToThread(&thread);
}

SingleThreadExecutor::~SingleThreadExecutor() {
  thread.quit();
  thread.wait();
}

void SingleThreadExecutor::ScheduleImpl(std::unique_ptr<Action> action) {
  QMetaObject::invokeMethod(
      &context,
      [action = std::move(action)]() {
        ORBIT_SCOPE("SingleThreadExecutor Action");
        action->Execute();
      },
      Qt::QueuedConnection);
}

}  // namespace orbit_qt_utils