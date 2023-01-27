// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_TEST_UTILS_WAIT_FOR_WITH_TIMEOUT_H_
#define QT_TEST_UTILS_WAIT_FOR_WITH_TIMEOUT_H_

#include <absl/time/time.h>
#include <gmock/gmock.h>

#include <chrono>

#include "OrbitBase/Result.h"
#include "QtUtils/CreateTimeout.h"
#include "QtUtils/WaitFor.h"

namespace orbit_qt_test_utils {

template <typename T>
orbit_qt_utils::TimeoutOr<T> WaitForWithTimeout(
    const orbit_base::Future<T>& future,
    std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}) {
  return orbit_qt_utils::WaitFor(orbit_qt_utils::WhenValueOrTimeout(future, timeout));
}

template <typename T>
orbit_qt_utils::TimeoutOr<T> WaitForWithTimeout(const orbit_base::Future<T>& future,
                                                absl::Duration timeout) {
  return orbit_qt_utils::WaitFor(orbit_qt_utils::WhenValueOrTimeout(future, timeout));
}

MATCHER(YieldsTimeout, "") {
  if (arg.has_error()) return true;
  *result_listener << "Error: Expected a timeout, but the WaitFor call yielded a result.";
  return false;
}

MATCHER(YieldsNoTimeout, "") {
  if (!arg.has_error()) return true;
  *result_listener << "Error: Expected no timeout, but a timeout occured.";
  return false;
}

MATCHER_P(YieldsResult, value_matcher, "") {
  if (!arg.has_value()) {
    *result_listener << "Error: Expected value, but the WaitFor call timed out.";
    return false;
  }
  return testing::ExplainMatchResult(value_matcher, arg.value(), result_listener);
}

// This helper function simplifies interaction with OUTCOME_TRY. It allows to convert a
// WaitForResult that timed out into a generic error message. That's useful if you don't need to
// distinguish between a time out and any other error.
template <typename T>
ErrorMessageOr<T> ConsiderTimeoutAnError(orbit_qt_utils::TimeoutOr<T>&& result) {
  if (result.has_value()) return result.value();
  return ErrorMessage{result.error().message()};
}

// This overload avoids ErrorMessageOr<ErrorMessageOr<T>> double wrapping. Unfortunately there is no
// good way to avoid the code deduplication that doesn't make it a lot more complex.
template <typename T>
ErrorMessageOr<T> ConsiderTimeoutAnError(orbit_qt_utils::TimeoutOr<ErrorMessageOr<T>>&& result) {
  if (result.has_value()) return result.value();
  return ErrorMessage{std::string{result.error().message()}};
}

}  // namespace orbit_qt_test_utils

#endif  // QT_TEST_UTILS_WAIT_FOR_WITH_TIMEOUT_H_