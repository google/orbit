// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/FutureWatcher.h"

#include <QEventLoop>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <algorithm>
#include <chrono>

#include "OrbitBase/WhenAll.h"

namespace orbit_qt_utils {

FutureWatcher::FutureWatcher(QObject* parent) : QObject{parent} {}

void FutureWatcher::Abort() { emit AbortRequested(); }

FutureWatcher::Reason FutureWatcher::WaitFor(
    const orbit_base::Future<void>& future,
    std::optional<std::chrono::milliseconds> timeout) const {
  if (!future.IsValid() || future.IsFinished()) return Reason::kFutureCompleted;

  QTimer timer{};
  timer.setSingleShot(true);

  QEventLoop loop{};
  QObject::connect(this, &FutureWatcher::AbortRequested, &loop, &QEventLoop::quit);
  QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

  if (timeout.has_value()) {
    timer.start(timeout.value());
  }

  const orbit_base::FutureRegisterContinuationResult result = future.RegisterContinuation(
      [loop = QPointer<QEventLoop>{&loop}] { QMetaObject::invokeMethod(loop, &QEventLoop::quit); });

  if (result != orbit_base::FutureRegisterContinuationResult::kSuccessfullyRegistered) {
    return Reason::kFutureCompleted;
  }

  loop.exec();

  if (future.IsFinished()) return Reason::kFutureCompleted;

  constexpr int kInvalidRemainingTime = -1;
  if (timeout.has_value() && timer.remainingTime() == kInvalidRemainingTime) {
    return Reason::kTimeout;
  }

  return Reason::kAbortRequested;
}

FutureWatcher::Reason FutureWatcher::WaitForAll(
    absl::Span<orbit_base::Future<void>> futures,
    std::optional<std::chrono::milliseconds> timeout) const {
  return WaitFor(orbit_base::WhenAll(futures), timeout);
}
}  // namespace orbit_qt_utils