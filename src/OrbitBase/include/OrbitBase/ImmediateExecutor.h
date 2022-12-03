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
// main_thread_executor_->WaitFor(void_future); // WaitFor only works with Future<void>
//
class ImmediateExecutor {
 public:
  template <typename F>
  auto Schedule(F&& invocable) {
    using ReturnType = decltype(invocable());
    if constexpr (std::is_same_v<ReturnType, void>) {
      invocable();
      return Future<void>{};
    } else {
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
    return resulting_future;
  }

  template <typename T, typename F>
  auto ScheduleAfterIfSuccess(const Future<ErrorMessageOr<T>>& future, F&& invocable) {
    ORBIT_CHECK(future.IsValid());

    using ResultType = typename ContinuationReturnType<T, F>::Type;
    using ReturnType = typename EnsureWrappedInErrorMessageOr<ResultType>::Type;
    orbit_base::Promise<ReturnType> promise{};
    orbit_base::Future<ReturnType> resulting_future = promise.GetFuture();

    auto continuation = [invocable = std::forward<F>(invocable),
                         promise = std::move(promise)](const ErrorMessageOr<T>& result) mutable {
      HandleErrorAndSetResultInPromise<ReturnType> helper{&promise};
      helper.Call(invocable, result);
    };

    orbit_base::RegisterContinuationOrCallDirectly(future, std::move(continuation));
    return resulting_future;
  }
};
}  // namespace orbit_base

#endif  // ORBIT_BASE_IMMEDIATE_EXECUTOR_H_