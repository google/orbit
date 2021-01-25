// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_MAIN_THREAD_EXECUTOR_IMPL_H_
#define ORBIT_QT_MAIN_THREAD_EXECUTOR_IMPL_H_

#include <QObject>
#include <memory>

#include "MainThreadExecutor.h"

namespace orbit_qt {

// An implementation of MainThreadExecutor that integrates with Qt's event loop
class MainThreadExecutorImpl : public QObject, public MainThreadExecutor {
  Q_OBJECT
  explicit MainThreadExecutorImpl(QObject* parent = nullptr) : QObject(parent) {}

 public:
  [[nodiscard]] static std::shared_ptr<MainThreadExecutorImpl> Create() {
    return std::shared_ptr<MainThreadExecutorImpl>{new MainThreadExecutorImpl{}};
  }

  using MainThreadExecutor::Schedule;
  void Schedule(std::unique_ptr<Action> action) override;

  WaitResult WaitFor(const orbit_base::Future<void>& future,
                     std::chrono::milliseconds timeout) override;

  WaitResult WaitFor(const orbit_base::Future<void>& future) override;

  WaitResult WaitForAll(absl::Span<orbit_base::Future<void>> futures,
                        std::chrono::milliseconds timeout) override;

  WaitResult WaitForAll(absl::Span<orbit_base::Future<void>> futures) override;

  void AbortWaitingJobs() override;
 signals:
  void AbortRequested();
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_MAIN_THREAD_EXECUTOR_IMPL_H_
