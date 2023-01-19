// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_CREATE_TIMEOUT_H_
#define QT_UTILS_CREATE_TIMEOUT_H_

#include <absl/time/time.h>

#include <QTimer>
#include <chrono>
#include <string_view>
#include <variant>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/VoidToMonostate.h"
#include "OrbitBase/WhenAny.h"

namespace orbit_qt_utils {

// This simple error type indicates a passed Timeout. See `CreateTimeout` and `WhenValueOrTimeout`
// below on how to make use of it.
struct Timeout {
  [[nodiscard]] constexpr static std::string_view message() { return "The operation timed out."; }
};

// Returns a future that completes at the earliest when `duration` has passed. Note that the timeout
// is checked by the current thread's event loop. If that event loop is busy, this timeout will also
// not expire.
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

// Convenience overload of the previous function that takes an `absl::Duration` instead.
[[nodiscard]] inline orbit_base::Future<Timeout> CreateTimeout(absl::Duration duration) {
  return CreateTimeout(absl::ToChronoMilliseconds(duration));
}

template <typename T>
using TimeoutOr = Result<T, Timeout>;

// This is a helper function to `CreateTimeout`. It returns a future that completes when either the
// given future `value` completes or when `duration` passes - whatever happens first.
template <typename T>
orbit_base::Future<TimeoutOr<T>> WhenValueOrTimeout(const orbit_base::Future<T>& value,
                                                    std::chrono::milliseconds duration) {
  orbit_base::ImmediateExecutor executor{};
  return orbit_base::WhenAny(value, CreateTimeout(duration))
      .Then(&executor,
            [](const std::variant<orbit_base::VoidToMonostate_t<T>, Timeout>& result)
                -> TimeoutOr<T> {
              if (result.index() == 1) return outcome::failure(std::get<1>(result));
              if constexpr (std::is_same_v<T, void>) {
                return outcome::success();
              } else {
                return outcome::success(std::get<0>(result));
              }
            });
}

// Convenience overload of the previous function that takes an `absl::Duration` instead.
template <typename T>
orbit_base::Future<TimeoutOr<T>> WhenValueOrTimeout(const orbit_base::Future<T>& value,
                                                    absl::Duration duration) {
  return WhenValueOrTimeout(value, absl::ToChronoMilliseconds(duration));
}

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_CREATE_TIMEOUT_H_