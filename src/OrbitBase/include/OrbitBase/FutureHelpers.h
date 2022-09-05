// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_FUTURE_HELPERS_H_
#define ORBIT_BASE_FUTURE_HELPERS_H_

#include <type_traits>
#include <utility>

#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"

namespace orbit_base {

template <typename T, typename Invocable>
void RegisterContinuationOrCallDirectly(const Future<T>& future, Invocable&& continuation) {
  const auto result = future.RegisterContinuation(std::forward<Invocable>(continuation));

  if (result == FutureRegisterContinuationResult::kFutureAlreadyCompleted) {
    continuation(future.Get());
  }
}

template <typename Invocable>
void RegisterContinuationOrCallDirectly(const Future<void>& future, Invocable&& continuation) {
  const auto result = future.RegisterContinuation(std::forward<Invocable>(continuation));

  if (result == FutureRegisterContinuationResult::kFutureAlreadyCompleted) {
    continuation();
  }
}

template <typename T>
[[nodiscard]] inline Future<T> UnwrapFuture(const Future<T>& future) {
  return future;
}

template <typename T>
[[nodiscard]] inline Future<T> UnwrapFuture(const Future<Future<T>>& outer_future) {
  ORBIT_CHECK(outer_future.IsValid());

  // A quick explanation what's happening here:
  // We have an outer and an inner future. When the outer future completes, the inner
  // one gets available. When the inner one completes the value of type T gets available.
  //
  // Ideally we would just return the inner future. But since it is not available at the time this
  // function is executed that is not an option. So we create a new promise/future pair.
  // Then we register a continuation with the inner future that sets the result in the just created
  // promise. That's also not so simple because - as said before - the inner future is not yet
  // available. So we have to register an outer continuation that takes care of registering the
  // inner continuation.
  auto promise = std::make_shared<Promise<T>>();

  auto outer_continuation = [promise](const Future<T>& inner_future) {
    auto inner_continuation = [promise](const T& arg) { promise->SetResult(arg); };
    RegisterContinuationOrCallDirectly(inner_future, inner_continuation);
  };
  RegisterContinuationOrCallDirectly(outer_future, outer_continuation);
  return promise->GetFuture();
}

[[nodiscard]] inline Future<void> UnwrapFuture(const Future<Future<void>>& outer_future) {
  ORBIT_CHECK(outer_future.IsValid());

  auto promise = std::make_shared<Promise<void>>();

  auto outer_continuation = [promise](const Future<void>& inner_future) {
    auto inner_continuation = [promise]() { promise->MarkFinished(); };
    RegisterContinuationOrCallDirectly(inner_future, inner_continuation);
  };
  RegisterContinuationOrCallDirectly(outer_future, outer_continuation);
  return promise->GetFuture();
}

template <typename T>
[[nodiscard]] Future<ErrorMessageOr<T>> UnwrapFuture(
    const Future<ErrorMessageOr<Future<T>>>& outer_future) {
  orbit_base::Promise<ErrorMessageOr<T>> promise{};
  auto future = promise.GetFuture();

  auto outer_continuation =
      [promise = std::move(promise)](const ErrorMessageOr<Future<T>>& outer_result) mutable {
        if (outer_result.has_error()) {
          promise.SetResult(outcome::failure(outer_result.error()));
          return;
        }

        auto inner_continuation = [promise = std::move(promise)](const T& arg) mutable {
          promise.SetResult(arg);
        };

        const auto& inner_future = outer_result.value();
        RegisterContinuationOrCallDirectly(inner_future, std::move(inner_continuation));
      };
  RegisterContinuationOrCallDirectly(outer_future, std::move(outer_continuation));

  return future;
}

template <>
[[nodiscard]] inline Future<ErrorMessageOr<void>> UnwrapFuture(
    const Future<ErrorMessageOr<Future<void>>>& outer_future) {
  orbit_base::Promise<ErrorMessageOr<void>> promise{};
  auto future = promise.GetFuture();

  auto outer_continuation =
      [promise = std::move(promise)](const ErrorMessageOr<Future<void>>& outer_result) mutable {
        if (outer_result.has_error()) {
          promise.SetResult(outcome::failure(outer_result.error()));
          return;
        }

        auto inner_continuation = [promise = std::move(promise)]() mutable {
          promise.SetResult(outcome::success());
        };

        const auto& inner_future = outer_result.value();
        RegisterContinuationOrCallDirectly(inner_future, std::move(inner_continuation));
      };
  RegisterContinuationOrCallDirectly(outer_future, std::move(outer_continuation));

  return future;
}

template <typename T>
[[nodiscard]] Future<ErrorMessageOr<T>> UnwrapFuture(
    const Future<ErrorMessageOr<Future<ErrorMessageOr<T>>>>& outer_future) {
  orbit_base::Promise<ErrorMessageOr<T>> promise{};
  auto future = promise.GetFuture();

  auto outer_continuation =
      [promise = std::move(promise)](
          const ErrorMessageOr<Future<ErrorMessageOr<T>>>& outer_result) mutable {
        if (outer_result.has_error()) {
          promise.SetResult(outcome::failure(outer_result.error()));
          return;
        }

        auto inner_continuation = [promise =
                                       std::move(promise)](const ErrorMessageOr<T>& arg) mutable {
          promise.SetResult(arg);
        };

        const auto& inner_future = outer_result.value();
        RegisterContinuationOrCallDirectly(inner_future, std::move(inner_continuation));
      };
  RegisterContinuationOrCallDirectly(outer_future, std::move(outer_continuation));

  return future;
}
}  // namespace orbit_base

#endif  // ORBIT_BASE_FUTURE_HELPERS_H_