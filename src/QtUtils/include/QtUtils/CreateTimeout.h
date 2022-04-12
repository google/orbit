// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_CREATE_TIMEOUT_H_
#define QT_UTILS_CREATE_TIMEOUT_H_

#include <absl/time/time.h>

#include <QTimer>
#include <chrono>
#include <variant>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"

namespace orbit_qt_utils {

// This helper type indicates an occured Timeout and is returned - wrapped in a future - by
// `CreateTimeout`. It doesn't hold any data. It's most useful in conjunction with
// `orbit_base::WhenAny` which returns a `std::variant` (wrapped in a future). You can determine
// whether a timeout occured by checking whether it holds a value of type `Timeout`
struct Timeout {};

// Returns a future that completes at the earliest when `duration` has passed.
[[nodiscard]] inline orbit_base::Future<Timeout> CreateTimeout(std::chrono::milliseconds duration) {
  orbit_base::Promise<Timeout> promise{};
  orbit_base::Future<Timeout> future = promise.GetFuture();

  // We need to use a Qt::PreciseTimer because only that guarantees to never wake up earlier than
  // `duration`. Other types might wake up up to 5% too early. We are also adding an additional
  // millisecond to the duration because the Qt::PreciseTimer only guarantees 1 millisecond
  // accuracy so it might still be early by less than a millisecond. Adding an additional
  // millisecond ensures that we keep the promise of not completing before the duration has passed.
  // Reference: https://doc.qt.io/qt-5/qtimer.html
  QTimer::singleShot(duration + std::chrono::milliseconds{1}, Qt::PreciseTimer,
                     [promise = std::move(promise)]() mutable { promise.SetResult(Timeout{}); });

  return future;
}

// Returns a future that completes at the earliest when `duration` has passed.
[[nodiscard]] inline orbit_base::Future<Timeout> CreateTimeout(absl::Duration duration) {
  return CreateTimeout(absl::ToChronoMilliseconds(duration));
}

// This is a helper type alias that is useful in combination with WhenAny. It provides a nicer way
// to write the value type of the future that is returned by WhenAny when given an arbitrary future
// and a Future<Timeout>.
template <typename T>
using ValueOrTimeout = std::variant<T, Timeout>;

// This is a helper type alias that is useful in combination with WhenAny. It provides a nicer way
// to write the return type of WhenAny when given an arbitrary future and a Future<Timeout>.
template <typename T>
using ValueOrTimeoutFuture = orbit_base::Future<std::variant<T, Timeout>>;

// This function helps to checks whether the timeout has completed first when joined with another
// future using `WhenAny`.
template <typename T>
[[nodiscard]] bool HasTimedOut(const std::variant<T, Timeout>& value_or_timeout) {
  return value_or_timeout.index() == 1;
}

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_CREATE_TIMEOUT_H_