// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/MainThreadExecutorImpl.h"

#include <QMetaObject>
#include <Qt>
#include <chrono>
#include <optional>
#include <type_traits>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/Action.h"
#include "OrbitBase/Logging.h"
#include "QtUtils/FutureWatcher.h"

using orbit_base::MainThreadExecutor;

namespace orbit_qt_utils {

void MainThreadExecutorImpl::ScheduleImpl(std::unique_ptr<Action> action) {
  QMetaObject::invokeMethod(
      this,
      [action = std::move(action)]() {
        ORBIT_SCOPE("MainThreadExecutor Action");
        action->Execute();
      },
      Qt::QueuedConnection);
}

void MainThreadExecutorImpl::AbortWaitingJobs() { emit AbortRequested(); }

MainThreadExecutor::WaitResult MapToWaitResult(orbit_qt_utils::FutureWatcher::Reason result) {
  using Reason = orbit_qt_utils::FutureWatcher::Reason;
  using WaitResult = MainThreadExecutor::WaitResult;

  switch (result) {
    case Reason::kFutureCompleted:
      return WaitResult::kCompleted;
    case Reason::kAbortRequested:
      return WaitResult::kAborted;
    case Reason::kTimeout:
      return WaitResult::kTimedOut;
  }

  ORBIT_UNREACHABLE();
}

MainThreadExecutor::WaitResult MainThreadExecutorImpl::WaitFor(
    const orbit_base::Future<void>& future, std::chrono::milliseconds timeout) {
  orbit_qt_utils::FutureWatcher watcher{};
  QObject::connect(this, &MainThreadExecutorImpl::AbortRequested, &watcher,
                   &orbit_qt_utils::FutureWatcher::Abort);
  const auto result = watcher.WaitFor(future, timeout);
  return MapToWaitResult(result);
}

MainThreadExecutor::WaitResult MainThreadExecutorImpl::WaitFor(
    const orbit_base::Future<void>& future) {
  orbit_qt_utils::FutureWatcher watcher{};
  QObject::connect(this, &MainThreadExecutorImpl::AbortRequested, &watcher,
                   &orbit_qt_utils::FutureWatcher::Abort);
  const auto result = watcher.WaitFor(future, std::nullopt);
  return MapToWaitResult(result);
}

MainThreadExecutor::WaitResult MainThreadExecutorImpl::WaitForAll(
    absl::Span<orbit_base::Future<void>> futures, std::chrono::milliseconds timeout) {
  orbit_qt_utils::FutureWatcher watcher{};
  QObject::connect(this, &MainThreadExecutorImpl::AbortRequested, &watcher,
                   &orbit_qt_utils::FutureWatcher::Abort);
  const auto result = watcher.WaitForAll(futures, timeout);
  return MapToWaitResult(result);
}

MainThreadExecutor::WaitResult MainThreadExecutorImpl::WaitForAll(
    absl::Span<orbit_base::Future<void>> futures) {
  orbit_qt_utils::FutureWatcher watcher{};
  QObject::connect(this, &MainThreadExecutorImpl::AbortRequested, &watcher,
                   &orbit_qt_utils::FutureWatcher::Abort);
  const auto result = watcher.WaitForAll(futures, std::nullopt);
  return MapToWaitResult(result);
}

}  // namespace orbit_qt_utils