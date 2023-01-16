// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/MainThreadExecutor.h"

#include <QMetaObject>
#include <Qt>
#include <type_traits>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/Action.h"

namespace orbit_qt_utils {

void MainThreadExecutor::ScheduleImpl(std::unique_ptr<Action> action) {
  QMetaObject::invokeMethod(
      this,
      [action = std::move(action)]() {
        ORBIT_SCOPE("MainThreadExecutor Action");
        action->Execute();
      },
      Qt::QueuedConnection);
}

}  // namespace orbit_qt_utils