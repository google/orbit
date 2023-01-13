// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_SINGLE_THREAD_EXECUTOR_H_
#define QT_UTILS_SINGLE_THREAD_EXECUTOR_H_

#include <QObject>
#include <QString>
#include <QThread>
#include <memory>

#include "OrbitBase/Action.h"
#include "OrbitBase/Executor.h"

namespace orbit_qt_utils {

// An implementation of an Executor that schedules tasks on a background thread. Only use this
// executor when you need your tasks to run on a `QThread`. If not, using the global instance of
// `orbit_base::ThreadPool` is the better option.
class SingleThreadExecutor : public QObject, public orbit_base::Executor {
  Q_OBJECT

 public:
  explicit SingleThreadExecutor(QObject* parent = nullptr);
  ~SingleThreadExecutor() override;

  [[nodiscard]] const QThread* GetThread() const { return &thread_; }
  [[nodiscard]] QThread* GetThread() { return &thread_; }

  [[nodiscard]] Handle GetExecutorHandle() const override { return executor_handle_.Get(); }

 private:
  void ScheduleImpl(std::unique_ptr<Action> action) override;

  QThread thread_{};
  QObject context_{};
  ScopedHandle executor_handle_{this};
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_SINGLE_THREAD_EXECUTOR_H_
