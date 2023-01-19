// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_IMMEDIATE_EXECUTOR_H_
#define ORBIT_BASE_IMMEDIATE_EXECUTOR_H_

#include <type_traits>

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/PromiseHelpers.h"

namespace orbit_base {

// ImmediateExecutor is a special type of executor that executes its action during the Schedule or
// ScheduleAfter call. When called directly it's just a verbose abstraction around a standard
// function call.
//
// Its usage only makes sense in combination with Future::Then.
//
// Example:
// Future<std::string> result = thread_pool_->Schedule(/* whatever */);
// ImmediateExecutor immediate_executor{};
// Future<void> void_future = result.Then(&immediate_executor, [](std::string_view str) {
//   (void) str;
// });
//
class ImmediateExecutor {
 public:
  template <typename F>
  auto Schedule(F&& invocable) {
    using ReturnType = decltype(invocable());
    if constexpr (std::is_same_v<ReturnType, void>) {
      invocable();
      return Future<void>{};
    } else if constexpr (orbit_base::kIsFutureV<ReturnType>) {
      // If the `invocable` returns a future we can just return that.
      return invocable();
    } else {
      // Otherwise we wrap the result into a future.
      return Future<ReturnType>{invocable()};
    }
  }

  template <typename T, typename F>
  auto ScheduleAfter(const Future<T>& future, F&& invocable) {
    ORBIT_CHECK(future.IsValid());

    using ReturnType = typename ContinuationReturnType<T, F>::Type;
    orbit_base::Promise<ReturnType> promise{};
    orbit_base::Future<ReturnType> resulting_future = promise.GetFuture();

    auto continuation = [invocable = std::forward<F>(invocable),
                         promise = std::move(promise)](auto&&... argument) mutable {
      orbit_base::CallTaskAndSetResultInPromise<ReturnType> helper{&promise};
      helper.Call(invocable, argument...);
    };

    orbit_base::RegisterContinuationOrCallDirectly(future, std::move(continuation));
    return UnwrapFuture(resulting_future);
  }

  template <typename T, typename E, typename F>
  auto ScheduleAfterIfSuccess(const Future<Result<T, E>>& future, F&& invocable) {
    ORBIT_CHECK(future.IsValid());

    using ResultType = typename ContinuationReturnType<T, F>::Type;
    using ReturnType = typename EnsureWrappedInResult<ResultType, E>::Type;
    orbit_base::Promise<ReturnType> promise{};
    orbit_base::Future<ReturnType> resulting_future = promise.GetFuture();

    auto continuation = [invocable = std::forward<F>(invocable),
                         promise = std::move(promise)](const Result<T, E>& result) mutable {
      HandleErrorAndSetResultInPromise<ReturnType> helper{&promise};
      helper.Call(invocable, result);
    };

    orbit_base::RegisterContinuationOrCallDirectly(future, std::move(continuation));
    return UnwrapFuture(resulting_future);
  }
};
}  // namespace orbit_base

#endif  // ORBIT_BASE_IMMEDIATE_EXECUTOR_H_