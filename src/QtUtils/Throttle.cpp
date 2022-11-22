// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtUtils/Throttle.h"

#include <chrono>
#include <ratio>

namespace orbit_qt_utils {

Throttle::Throttle(std::chrono::milliseconds interval, QObject* parent)
    : QObject(parent), interval_{interval} {
  timer_.setSingleShot(true);
  QObject::connect(&timer_, &QTimer::timeout, this, &Throttle::TriggerNow);
}

void Throttle::Fire() {
  if (timer_.isActive()) {
    // Timer already running - so we just consume this call to Fire.
    return;
  }

  if (!last_time_executed_.has_value()) {
    // The Throttle has never been triggered before, so lets trigger right away.
    TriggerNow();
    return;
  }

  const std::chrono::steady_clock::duration elapsed_time =
      std::chrono::steady_clock::now() - last_time_executed_.value();

  if (elapsed_time >= interval_) {
    // The previous trigger was more than `interval_` ago. Let's trigger right away.
    TriggerNow();
    return;
  }

  const std::chrono::milliseconds remaining_time =
      interval_ - std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);

  // We have to wait before we can trigger the next time. Let's start the timer.
  timer_.start(remaining_time);
}

void Throttle::TriggerNow() {
  last_time_executed_ = std::chrono::steady_clock::now();
  emit Triggered();
}
}  // namespace orbit_qt_utils