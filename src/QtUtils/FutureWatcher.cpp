// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/FutureWatcher.h"

#include <absl/synchronization/mutex.h>

#include <QDebug>
#include <QEventLoop>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <chrono>

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
      [loop = QPointer{&loop}] { QMetaObject::invokeMethod(loop, &QEventLoop::quit); });

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
  if (futures.empty()) return Reason::kFutureCompleted;

  QTimer timer{};
  timer.setSingleShot(true);

  QEventLoop loop{};
  QObject::connect(this, &FutureWatcher::AbortRequested, &loop, &QEventLoop::quit);
  QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

  if (timeout.has_value()) {
    timer.start(timeout.value());
  }

  struct SharedData {
    absl::Mutex mutex;
    size_t completion_counter{};
    std::vector<bool> finished_indicator;
  };

  auto shared_data = std::make_shared<SharedData>();
  shared_data->finished_indicator.resize(futures.size(), false);
  shared_data->completion_counter = futures.size();  // Counts downwards

  auto indicator_iterator = shared_data->finished_indicator.begin();
  for (auto& future : futures) {
    if (!future.IsValid() || future.IsFinished()) {
      absl::MutexLock lock{&shared_data->mutex};
      --shared_data->completion_counter;
      *indicator_iterator = true;
    } else {
      const orbit_base::FutureRegisterContinuationResult result =
          future.RegisterContinuation([shared_data, indicator_iterator, loop = QPointer{&loop}]() {
            absl::MutexLock lock{&shared_data->mutex};

            if (shared_data->completion_counter == 0) return;

            if (!*indicator_iterator) {
              *indicator_iterator = true;
              --shared_data->completion_counter;
            }

            if (shared_data->completion_counter == 0) {
              QMetaObject::invokeMethod(loop, &QEventLoop::quit);
            }
          });

      if (result == orbit_base::FutureRegisterContinuationResult::kFutureAlreadyCompleted) {
        absl::MutexLock lock{&shared_data->mutex};
        if (!*indicator_iterator) {
          --shared_data->completion_counter;
          *indicator_iterator = true;
        }
      }
    }

    ++indicator_iterator;
  }

  {
    absl::MutexLock lock{&shared_data->mutex};
    if (shared_data->completion_counter == 0) return Reason::kFutureCompleted;
  }

  loop.exec();

  {
    absl::MutexLock lock{&shared_data->mutex};
    if (shared_data->completion_counter == 0) return Reason::kFutureCompleted;

    // If we reach this point, it means the waiting was cancelled (either by Abort or by timeout).
    // We set completion_counter to zero to inform later executing continuations that we are done.
    shared_data->completion_counter = 0;
  }

  constexpr int kInvalidRemainingTime = -1;
  if (timeout && timer.remainingTime() == kInvalidRemainingTime) return Reason::kTimeout;

  return Reason::kAbortRequested;
}
}  // namespace orbit_qt_utils