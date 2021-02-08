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
  CHECK(outer_future.IsValid());

  // A quick explanation what's happening here:
  // We have an outer and a inner future. When the outer future completes, the inner
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
  CHECK(outer_future.IsValid());

  auto promise = std::make_shared<Promise<void>>();

  auto outer_continuation = [promise](const Future<void>& inner_future) {
    auto inner_continuation = [promise]() { promise->MarkFinished(); };
    RegisterContinuationOrCallDirectly(inner_future, inner_continuation);
  };
  RegisterContinuationOrCallDirectly(outer_future, outer_continuation);
  return promise->GetFuture();
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_FUTURE_HELPERS_H_