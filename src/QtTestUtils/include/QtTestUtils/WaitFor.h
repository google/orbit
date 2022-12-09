// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_TEST_UTILS_WAIT_FOR_H_
#define QT_TEST_UTILS_WAIT_FOR_H_

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>

#include <QTest>
#include <chrono>
#include <optional>
#include <variant>

#include "OrbitBase/Future.h"
#include "OrbitBase/VoidToMonostate.h"

namespace orbit_qt_test_utils_internal {

// This is a helper type indicating that a Timeout has occured in a WaitForResult<T>. You don't have
// to interact with it directly. Rather use the predicates 'HasTimedOut' and 'HasValue'.
struct TimeoutOccured {};

constexpr int kWaitForResultTimeoutIndex = 0;
constexpr int kWaitForResultValueIndex = 1;

}  // namespace orbit_qt_test_utils_internal

namespace orbit_qt_test_utils {

// That's the return value of the below defined `WaitFor` function. It either indicates a timeout or
// holds a value that has been returned by the future. To check its state you can use the predicates
// `HasValue` and `HasTimedOut`. There is also a `GetValue` function which extracts the value in
// case the result holds a value.
template <typename T>
using WaitForResult = std::variant<orbit_qt_test_utils_internal::TimeoutOccured, T>;

// Takes an orbit_base::Future and waits until it completes or it times out. While waiting a Qt
// event loop is processing events in the background. The timeout duration can be adjusted with the
// second parameter.
//
// The return type is a WaitForResult that either contains the future's return value (in
// case the future completed) or indicates a timeout. You can use the predicates `HasTimedOut` and
// `HasValue` to check its state. There are also GMock matchers `YieldsResult(value_matcher)`,
// `YieldsNoTimeout()`, and `YieldsTimeout()`.
template <typename T>
[[nodiscard]] WaitForResult<orbit_base::VoidToMonostate_t<T>> WaitFor(
    const orbit_base::Future<T>& future,
    std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}) {
  if (QTest::qWaitFor([&future]() { return future.IsFinished(); },
                      static_cast<int>(timeout.count()))) {
    if constexpr (std::is_same_v<T, void>) {
      return std::monostate{};
    } else {
      return future.Get();
    }
  }
  return orbit_qt_test_utils_internal::TimeoutOccured{};
}

template <typename T>
[[nodiscard]] bool HasTimedOut(const WaitForResult<T>& result) {
  return result.index() == orbit_qt_test_utils_internal::kWaitForResultTimeoutIndex;
}

template <typename T>
[[nodiscard]] bool HasValue(const WaitForResult<T>& result) {
  return result.index() == orbit_qt_test_utils_internal::kWaitForResultValueIndex;
}

template <typename T>
[[nodiscard]] std::optional<T> GetValue(const WaitForResult<T>& result) {
  static_assert(!std::is_same_v<T, std::monostate>,
                "Calling GetValue on a WaitForResult of a Future<void> makes no sense. You can use "
                "'HasValue' to see if it succeeded.");
  if (result.index() == orbit_qt_test_utils_internal::kWaitForResultValueIndex) {
    return std::get<orbit_qt_test_utils_internal::kWaitForResultValueIndex>(result);
  }

  return std::nullopt;
}

template <typename T>
[[nodiscard]] std::optional<T> GetValue(WaitForResult<T>&& result) {
  static_assert(!std::is_same_v<T, std::monostate>,
                "Calling GetValue on a WaitForResult of a Future<void> makes no sense. You can use "
                "'HasValue' to see if it succeeded.");
  if (std::holds_alternative<orbit_qt_test_utils_internal::kWaitForResultValueIndex>(result)) {
    return std::get<orbit_qt_test_utils_internal::kWaitForResultValueIndex>(std::move(result));
  }

  return std::nullopt;
}

MATCHER(YieldsTimeout, "") {
  if (HasTimedOut(arg)) return true;
  *result_listener << "Error: Expected a timeout, but the WaitFor call yielded a result.";
  return false;
}

MATCHER(YieldsNoTimeout, "") {
  if (!HasTimedOut(arg)) return true;
  *result_listener << "Error: Expected no timeout, but a timeout occured.";
  return false;
}

MATCHER_P(YieldsResult, value_matcher, "") {
  if (HasTimedOut(arg)) {
    *result_listener << "Error: Expected value, but the WaitFor call timed out.";
    return false;
  }
  return testing::ExplainMatchResult(
      value_matcher, std::get<orbit_qt_test_utils_internal::kWaitForResultValueIndex>(arg),
      result_listener);
}

}  // namespace orbit_qt_test_utils

#endif  // QT_TEST_UTILS_WAIT_FOR_H_