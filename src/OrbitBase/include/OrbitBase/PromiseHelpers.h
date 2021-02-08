// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROMISE_HELPERS_H_
#define ORBIT_BASE_PROMISE_HELPERS_H_

#include <type_traits>
#include <utility>

#include "OrbitBase/Promise.h"

namespace orbit_base {

// Calls `invocable` and sets the return value as the result in the given promise.
// It also works when `invocable` returns `void`. In this case `MarkFinished` is called on the
// promise.
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

}  // namespace orbit_base

#endif  // ORBIT_BASE_PROMISE_HELPERS_H_