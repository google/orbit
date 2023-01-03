// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_MAIN_THREAD_EXECUTOR_IMPL_H_
#define QT_UTILS_MAIN_THREAD_EXECUTOR_IMPL_H_

#include <absl/types/span.h>

#include <QObject>
#include <QString>
#include <chrono>
#include <memory>

#include "OrbitBase/Action.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/MainThreadExecutor.h"

namespace orbit_qt_utils {

// An implementation of MainThreadExecutor that integrates with Qt's event loop
class MainThreadExecutorImpl : public QObject, public orbit_base::MainThreadExecutor {
  Q_OBJECT

 public:
  explicit MainThreadExecutorImpl(QObject* parent = nullptr) : QObject(parent) {}

  WaitResult WaitFor(const orbit_base::Future<void>& future,
                     std::chrono::milliseconds timeout) override;

  WaitResult WaitFor(const orbit_base::Future<void>& future) override;

  WaitResult WaitForAll(absl::Span<orbit_base::Future<void>> futures,
                        std::chrono::milliseconds timeout) override;

  WaitResult WaitForAll(absl::Span<orbit_base::Future<void>> futures) override;

  void AbortWaitingJobs() override;

  [[nodiscard]] Handle GetExecutorHandle() const override { return executor_handle_.Get(); }

 signals:
  void AbortRequested();

 private:
  void ScheduleImpl(std::unique_ptr<Action> action) override;

  ScopedHandle executor_handle_{this};
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_MAIN_THREAD_EXECUTOR_IMPL_H_
