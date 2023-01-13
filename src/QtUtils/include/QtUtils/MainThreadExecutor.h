// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_MAIN_THREAD_EXECUTOR_H_
#define QT_UTILS_MAIN_THREAD_EXECUTOR_H_

#include <absl/types/span.h>

#include <QObject>
#include <QString>
#include <chrono>
#include <memory>

#include "OrbitBase/Action.h"
#include "OrbitBase/Executor.h"

namespace orbit_qt_utils {

// An Executor that runs tasks on the main thread.
//
// Notes:
// 1. Tasks are only executed when a Qt event loop is running.
// 2. This executor must be constructed on the main thread.
class MainThreadExecutor : public QObject, public orbit_base::Executor {
  Q_OBJECT

 public:
  explicit MainThreadExecutor(QObject* parent = nullptr) : QObject(parent) {}

  [[nodiscard]] Handle GetExecutorHandle() const override { return executor_handle_.Get(); }

 private:
  void ScheduleImpl(std::unique_ptr<Action> action) override;

  ScopedHandle executor_handle_{this};
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_MAIN_THREAD_EXECUTOR_H_
