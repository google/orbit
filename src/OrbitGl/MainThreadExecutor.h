// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MAIN_THREAD_EXECUTOR_H_
#define ORBIT_GL_MAIN_THREAD_EXECUTOR_H_

#include <absl/types/span.h>

#include <chrono>
#include <list>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>

#include "OrbitBase/Action.h"
#include "OrbitBase/AnyMovable.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/PromiseHelpers.h"

// This class implements a mechanism for landing
// actions to the main thread. As a general rule
// waiting on sockets and processing should be
// happening off the main thread and the main thread
// should only be responsible for updating user
// interface and models.
//
// Usage example:
// /* A caller who wants to process something on the main thread,
//  * note that this is not-blocking operation and will be processed
//  * at some time in the future on the main thread.
//  */
//
// manager->Schedule(CreateAction([data]{
//   UpdateSomethingWith(data);
// }));
class MainThreadExecutor : public std::enable_shared_from_this<MainThreadExecutor> {
 public:
  MainThreadExecutor() = default;
  virtual ~MainThreadExecutor() = default;

  // Schedules the action to be performed on the main thread.
  virtual void Schedule(std::unique_ptr<Action> action) = 0;

  // Schedule schedules the function object `functor` to be executed
  // on `*this`. The execution occures asynchronously, meaning this
  // function call will only push the function object to a queue. It
  // will be picked up by an event loop cycle.
  //
  // Note: The function object is only executed if `*this` is still alive when the event loop picks
  // up the scheduled task.
  template <typename F>
  auto Schedule(F&& functor) -> orbit_base::Future<std::decay_t<decltype(functor())>> {
    using ReturnType = std::decay_t<decltype(functor())>;

    orbit_base::Promise<ReturnType> promise;
    orbit_base::Future<ReturnType> future = promise.GetFuture();

    auto function_wrapper = [functor = std::forward<F>(functor),
                             promise = std::move(promise)]() mutable {
      orbit_base::CallTaskAndSetResultInPromise<ReturnType> helper{&promise};
      helper.Call(functor);
    };
    Schedule(CreateAction(std::move(function_wrapper)));

    return future;
  }

  // ScheduleAfter schedules the continuation `functor` to be executed
  // on `*this` after `future` has completed.
  //
  // Note: The continuation is only executed if `*this` is still alive when `future` completes.
  template <typename T, typename F>
  auto ScheduleAfter(const orbit_base::Future<T>& future, F&& functor) {
    CHECK(future.IsValid());

    using ReturnType = typename orbit_base::ContinuationReturnType<T, F>::Type;

    orbit_base::Promise<ReturnType> promise{};
    orbit_base::Future<ReturnType> resulting_future = promise.GetFuture();

    waiting_continuations_.emplace_front(std::forward<F>(functor));
    auto function_reference = waiting_continuations_.begin();

    auto continuation = [this, function_reference, executor_weak_ptr = weak_from_this(),
                         promise = std::move(promise)](auto&&... argument) mutable {
      auto executor = executor_weak_ptr.lock();
      if (executor == nullptr) return;

      auto function_wrapper =
          [this, function_reference, promise = std::move(promise),
           argument = std::make_tuple(std::forward<decltype(argument)>(argument)...)]() mutable {
            orbit_base::CallTaskAndSetResultInPromise<ReturnType> helper{&promise};

            // We don't have to worry about `function_reference` being invalid. It's a stable
            // iterator under the hood and this lambda will only be executed if the executor is
            // still alive.
            auto functor = orbit_base::any_movable_cast<std::decay_t<F>>(&*function_reference);
            CHECK(functor != nullptr);
            std::apply([&](auto... args) { helper.Call(*functor, args...); }, std::move(argument));
            waiting_continuations_.erase(function_reference);
          };
      executor->Schedule(CreateAction(std::move(function_wrapper)));
    };

    const orbit_base::FutureRegisterContinuationResult result =
        future.RegisterContinuation(std::move(continuation));

    if (result != orbit_base::FutureRegisterContinuationResult::kSuccessfullyRegistered) {
      // If the future has already been finished, we call the continuation here.
      // Keep in mind, this will not run the task synchronously. This will only
      // SCHEDULE the task synchronously.
      orbit_base::GetResultFromFutureAndCallContinuation<T> helper{&future};
      helper.Call(continuation);
    }

    return resulting_future;
  }

  // ScheduleAfterIfSuccess schedules the continuation `functor` to be executed
  // on `*this` after `future` has completed, and only if `future` returns a non-error result.
  //
  // Note: The continuation is only executed if `*this` is still alive when `future` completes.
  template <typename T, typename F>
  auto ScheduleAfterIfSuccess(const orbit_base::Future<ErrorMessageOr<T>>& future, F&& functor) {
    CHECK(future.IsValid());

    using ContinuationReturnType = typename orbit_base::ContinuationReturnType<T, F>::Type;
    using PromiseReturnType =
        typename orbit_base::EnsureWrappedInErrorMessageOr<ContinuationReturnType>::Type;

    orbit_base::Promise<PromiseReturnType> promise{};
    orbit_base::Future<PromiseReturnType> resulting_future = promise.GetFuture();

    waiting_continuations_.emplace_front(std::forward<F>(functor));
    auto function_reference = waiting_continuations_.begin();

    auto continuation = [this, function_reference, executor_weak_ptr = weak_from_this(),
                         promise = std::move(promise)](const ErrorMessageOr<T>& argument) mutable {
      auto executor = executor_weak_ptr.lock();
      if (executor == nullptr) return;

      // If the future returns a non-success ErrorMessageOr-type, we will short-circuit and won't
      // call the continutation. But we still have to schedule an action, that destroys the
      // continuation in the executor's context (think main thread). The continuation's destructor
      // might do things that need synchronization and can't happen in a different context (like a
      // thread pool), i.e. when the continuation owns a ScopedStatus.
      if (argument.has_error()) {
        promise.SetResult(argument.error());
        executor->Schedule(CreateAction(
            [this, function_reference]() { waiting_continuations_.erase(function_reference); }));
        return;
      }

      auto sucess_function_wrapper = [this, function_reference, promise = std::move(promise),
                                      argument]() mutable {
        auto functor = orbit_base::any_movable_cast<std::decay_t<F>>(&*function_reference);
        CHECK(functor != nullptr);

        orbit_base::HandleErrorAndSetResultInPromise<PromiseReturnType> helper{&promise};
        helper.Call(*functor, argument);

        waiting_continuations_.erase(function_reference);
      };
      executor->Schedule(CreateAction(std::move(sucess_function_wrapper)));
    };

    orbit_base::RegisterContinuationOrCallDirectly(future, std::move(continuation));
    return resulting_future;
  }

  enum class WaitResult { kCompleted, kTimedOut, kAborted };
  [[nodiscard]] virtual WaitResult WaitFor(const orbit_base::Future<void>& future,
                                           std::chrono::milliseconds timeout) = 0;

  [[nodiscard]] virtual WaitResult WaitFor(const orbit_base::Future<void>& future) = 0;

  [[nodiscard]] virtual WaitResult WaitForAll(absl::Span<orbit_base::Future<void>> futures,
                                              std::chrono::milliseconds timeout) = 0;

  [[nodiscard]] virtual WaitResult WaitForAll(absl::Span<orbit_base::Future<void>> futures) = 0;

  virtual void AbortWaitingJobs() = 0;

  [[nodiscard]] size_t GetNumberOfWaitingContinuations() const {
    return waiting_continuations_.size();
  }

 private:
  std::list<orbit_base::AnyMovable> waiting_continuations_;
};

template <typename F>
auto TrySchedule(const std::weak_ptr<MainThreadExecutor>& executor, F&& function_object)
    -> std::optional<decltype(std::declval<MainThreadExecutor>().Schedule(function_object))> {
  auto executor_ptr = executor.lock();
  if (executor_ptr == nullptr) return std::nullopt;

  return std::make_optional(executor_ptr->Schedule(std::forward<F>(function_object)));
}

#endif  // ORBIT_GL_MAIN_THREAD_EXECUTOR_H_
