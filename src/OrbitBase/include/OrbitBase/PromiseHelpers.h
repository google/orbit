// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROMISE_HELPERS_H_
#define ORBIT_BASE_PROMISE_HELPERS_H_

#include <type_traits>
#include <utility>

#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"

namespace orbit_base {

// Calls `invocable` and sets the return value as the result in the given promise.
// It also works when `invocable` returns `void`. In this case `MarkFinished` is called on the
// promise.
//
// The main benefit of this is the specialization for `T = void`, which requires different syntax.
// Don't use it if you don't need that particular feature.
template <typename T>
struct CallTaskAndSetResultInPromise {
  orbit_base::Promise<T>* promise;

  template <typename Invocable, typename... Args>
  void Call(Invocable&& invocable, Args&&... args) {
    promise->SetResult(invocable(std::forward<Args>(args)...));
  }
};

template <>
struct CallTaskAndSetResultInPromise<void> {
  orbit_base::Promise<void>* promise;

  template <typename Invocable, typename... Args>
  void Call(Invocable&& invocable, Args&&... args) {
    invocable(std::forward<Args>(args)...);
    promise->MarkFinished();
  }
};

// Calls `invocable` if the given ErrorMessageOr does not hold an error. The return value of
// `invocable` is set as the result in the given promise. The value in ErrorMessageOr<T> is passed
// as a parameter to `invocable`. If it is of type void, `invocable` is called without any
// arguments.
//
// The main benefit of this is the specialization for `T = void`, which requires different syntax.
// Don't use it if you don't need that particular feature.
template <typename R>
struct HandleErrorAndSetResultInPromise {
  orbit_base::Promise<R>* promise;

  template <typename Invocable, typename T>
  void Call(Invocable&& invocable, const ErrorMessageOr<T>& input) {
    if (input.has_error()) {
      promise->SetResult(outcome::failure(input.error()));
      return;
    }

    if constexpr (std::is_same_v<T, void>) {
      if constexpr (std::is_same_v<decltype(invocable()), void>) {
        invocable();
        promise->SetResult(outcome::success());
      } else if constexpr (IsErrorMessageOr<decltype(invocable())>::value) {
        promise->SetResult(invocable());
      } else {
        promise->SetResult(outcome::success(invocable()));
      }
    } else {
      if constexpr (std::is_same_v<decltype(invocable(input.value())), void>) {
        invocable(input.value());
        promise->SetResult(outcome::success());
      } else if constexpr (IsErrorMessageOr<decltype(invocable(input.value()))>::value) {
        promise->SetResult(invocable(input.value()));
      } else {
        promise->SetResult(outcome::success(invocable(input.value())));
      }
    }
  }
};

// Retrieves the result from a futures and calls `invocable` with the result as a parameter.
// It also works when the futures value type is void. In this case `invocable` is called without
// parameters.
template <typename T>
struct GetResultFromFutureAndCallContinuation {
  const orbit_base::Future<T>* future;

  template <typename Invocable>
  void Call(Invocable&& invocable) {
    invocable(future->Get());
  }
};

template <>
struct GetResultFromFutureAndCallContinuation<void> {
  const orbit_base::Future<void>* future;

  template <typename Invocable>
  void Call(Invocable&& invocable) {
    future->Wait();
    invocable();
  }
};

// Determines the return type of the call "Invocable(T)". If `T` is `void` it returns the return
// type of the call `Invocable()`.
template <typename T, typename Invocable>
struct ContinuationReturnType {
  using Type = std::decay_t<decltype(std::declval<Invocable>()(std::declval<T>()))>;
};

template <typename Invocable>
struct ContinuationReturnType<void, Invocable> {
  using Type = std::decay_t<decltype(std::declval<Invocable>()())>;
};

// This type trait helps to wrap a type into a ErrorMessageOr-wrapper, but won't do that if the type
// is already a ErrorMessageOr.
//
// Examples:
// EnsureWrappedInErrorMessageOr<int>::Type == ErrorMessageOr<int>
// EnsureWrappedInErrorMessageOr<ErrorMessageOr<int>>::Type == ErrorMessageOr<int>
template <typename T>
struct EnsureWrappedInErrorMessageOr {
  using Type = ErrorMessageOr<T>;
};

template <typename T>
struct EnsureWrappedInErrorMessageOr<ErrorMessageOr<T>> {
  using Type = ErrorMessageOr<T>;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_PROMISE_HELPERS_H_