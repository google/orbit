// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_EXECUTOR_H_
#define ORBIT_BASE_EXECUTOR_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <list>
#include <memory>
#include <optional>

#include "OrbitBase/Action.h"
#include "OrbitBase/AnyMovable.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/PromiseHelpers.h"

namespace orbit_base {

// Executor is a common base class for ThreadPool and MainThreadExecutor.
// Check out these two for details.
class Executor : public std::enable_shared_from_this<Executor> {
 public:
  // Handles are used by thread-safe asynchronous scheduling mechanims like `ScheduleAfter` or
  // `TrySchedule`. `Executor` assumes there is only one distinct instance of `Handle` (and an
  // arbitrary number of copies) per executor. `ScopedHandle` ensures that and simplifies handling
  // the executor implementation.
  class Handle final {
    struct HandleData {
      absl::Mutex mutex_;
      Executor* executor_ ABSL_GUARDED_BY(mutex_);

      explicit HandleData(Executor* executor) : executor_{executor} {}

      void InvalidateHandle() {
        absl::WriterMutexLock lock{&mutex_};
        executor_ = nullptr;
      }
    };

    explicit Handle(Executor* executor) : data_{std::make_shared<HandleData>(executor)} {}

    friend class Executor;

    template <typename F>
    friend auto TrySchedule(const Handle& handle, F&& function_object);

    std::shared_ptr<HandleData> data_;
  };

 protected:
  // `ScopedHandle` is a helper for implementations of `Executor`. It can and should be used to
  // manage an instance of `Handle` that can be returned by `GetExecutorHandle`.
  class ScopedHandle final {
   public:
    explicit ScopedHandle(Executor* executor) : handle_{executor} {}
    ~ScopedHandle() { InvalidateHandle(); }

    [[nodiscard]] Handle Get() const { return handle_; }
    void InvalidateHandle() { handle_.data_->InvalidateHandle(); }

   private:
    Handle handle_;
  };

 public:
  virtual ~Executor() = default;

  // Asynchronous scheduling mechanims like `ScheduleAfter` and `TrySchedule` require a stable
  // pointer of the executor. So we disable moving. And copying makes no sense either.
  Executor() = default;
  Executor(const Executor&) = delete;
  Executor& operator=(const Executor&) = delete;
  Executor(Executor&&) = delete;
  Executor& operator=(Executor&&) = delete;

  // Schedule schedules the function object `functor` to be executed on `*this`. The execution
  // occures asynchronously, meaning this function call will only push the function object to a
  // queue. It will be picked up by an event loop cycle.
  //
  // Note: The function object is only executed if `*this` is still alive when the event loop picks
  // up the scheduled task.
  template <typename F>
  auto Schedule(F&& functor) {
    using ReturnType = std::decay_t<decltype(functor())>;

    orbit_base::Promise<ReturnType> promise;
    orbit_base::Future<ReturnType> future = promise.GetFuture();

    auto function_wrapper = [functor = std::forward<F>(functor),
                             promise = std::move(promise)]() mutable {
      orbit_base::CallTaskAndSetResultInPromise<ReturnType> helper{&promise};
      helper.Call(functor);
    };
    ScheduleImpl(CreateAction(std::move(function_wrapper)));

    return UnwrapFuture(future);
  }

  // ScheduleAfter schedules the continuation `functor` to be executed on `*this` after `future` has
  // completed.
  //
  // Note: The continuation is only executed if `*this` is still alive when `future` completes.
  template <typename T, typename F>
  auto ScheduleAfter(const orbit_base::Future<T>& future, F&& functor) {
    ORBIT_CHECK(future.IsValid());

    using ReturnType = typename orbit_base::ContinuationReturnType<T, F>::Type;

    orbit_base::Promise<ReturnType> promise{};
    orbit_base::Future<ReturnType> resulting_future = promise.GetFuture();

    waiting_continuations_.emplace_front(std::forward<F>(functor));
    auto function_reference = waiting_continuations_.begin();

    auto continuation = [this, function_reference, executor_handle = GetExecutorHandle(),
                         promise = std::move(promise)](auto&&... argument) mutable {
      absl::ReaderMutexLock lock{&executor_handle.data_->mutex_};
      if (executor_handle.data_->executor_ == nullptr) return;

      auto function_wrapper =
          [this, function_reference, promise = std::move(promise),
           argument = std::make_tuple(std::forward<decltype(argument)>(argument)...)]() mutable {
            orbit_base::CallTaskAndSetResultInPromise<ReturnType> helper{&promise};

            // We don't have to worry about `function_reference` being invalid. It's a stable
            // iterator under the hood and this lambda will only be executed if the executor is
            // still alive.
            auto functor = orbit_base::any_movable_cast<std::decay_t<F>>(&*function_reference);
            ORBIT_CHECK(functor != nullptr);
            std::apply([&](auto... args) { helper.Call(*functor, args...); }, std::move(argument));
            waiting_continuations_.erase(function_reference);
          };
      executor_handle.data_->executor_->ScheduleImpl(CreateAction(std::move(function_wrapper)));
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

    return UnwrapFuture(resulting_future);
  }

  // ScheduleAfterIfSuccess schedules the continuation `functor` to be executed on `*this` after
  // `future` has completed, and only if `future` returns a non-error result.
  //
  // Note: The continuation is only executed if `*this` is still alive when `future` completes.
  template <typename T, typename E, typename F>
  auto ScheduleAfterIfSuccess(const orbit_base::Future<Result<T, E>>& future, F&& functor) {
    ORBIT_CHECK(future.IsValid());

    using ContinuationReturnType = typename orbit_base::ContinuationReturnType<T, F>::Type;
    using PromiseReturnType =
        typename orbit_base::EnsureWrappedInResult<ContinuationReturnType, E>::Type;

    orbit_base::Promise<PromiseReturnType> promise{};
    orbit_base::Future<PromiseReturnType> resulting_future = promise.GetFuture();

    waiting_continuations_.emplace_front(std::forward<F>(functor));
    auto function_reference = waiting_continuations_.begin();

    auto continuation = [this, function_reference, executor_handle = GetExecutorHandle(),
                         promise = std::move(promise)](const Result<T, E>& argument) mutable {
      // If the future returns a non-success ErrorMessageOr-type, we will short-circuit and won't
      // call the continuation. But we still have to schedule an action, that destroys the
      // continuation in the executor's context (think main thread). The continuation's destructor
      // might do things that need synchronization and can't happen in a different context (like a
      // thread pool), i.e. when the continuation owns a ScopedStatus.
      if (argument.has_error()) {
        // Here in SetResult other continuations might be called to propogate the result,
        // and some of these continuations might result in calling of ScheduleAfter() which locks
        // the same executor mutex. To avoid possible deadlocks, execute SetResult() here
        // in the non-locked context.
        promise.SetResult(outcome::failure(argument.error()));

        absl::ReaderMutexLock lock{&executor_handle.data_->mutex_};
        if (executor_handle.data_->executor_ == nullptr) return;

        executor_handle.data_->executor_->ScheduleImpl(CreateAction(
            [this, function_reference]() { waiting_continuations_.erase(function_reference); }));
        return;
      }

      auto success_function_wrapper = [this, function_reference, promise = std::move(promise),
                                       argument]() mutable {
        auto functor = orbit_base::any_movable_cast<std::decay_t<F>>(&*function_reference);
        ORBIT_CHECK(functor != nullptr);

        orbit_base::HandleErrorAndSetResultInPromise<PromiseReturnType> helper{&promise};
        helper.Call(*functor, argument);

        waiting_continuations_.erase(function_reference);
      };

      absl::ReaderMutexLock lock{&executor_handle.data_->mutex_};
      if (executor_handle.data_->executor_ == nullptr) return;

      executor_handle.data_->executor_->ScheduleImpl(
          CreateAction(std::move(success_function_wrapper)));
    };

    orbit_base::RegisterContinuationOrCallDirectly(future, std::move(continuation));
    return UnwrapFuture(resulting_future);
  }

  [[nodiscard]] size_t GetNumberOfWaitingContinuations() const {
    return waiting_continuations_.size();
  }

  // Executor implementations have to implement this getter using `ScopedHandle` from above. An
  // instance of `ScopedHandle` is supposed to be a member field of the executor implementation
  // (derived class). It is supposed to be destructed before the executor looses its ability to
  // schedule jobs. In most cases it should be enough to make the `ScopedHandle` the last member
  // field such that it will be destructed first. But it's also possible to call
  // `scoped_handle_.InvalidateHandle();` in the destructor or elsewhere explicitly.
  [[nodiscard]] virtual Handle GetExecutorHandle() const = 0;

 private:
  std::list<orbit_base::AnyMovable> waiting_continuations_;

  // Schedules the action to be performed on the executor.
  // Note for implementers: `ScheduleImpl` needs to be thread-safe!
  virtual void ScheduleImpl(std::unique_ptr<Action> action) = 0;
};

template <typename F>
auto TrySchedule(const Executor::Handle& handle, F&& function_object) {
  absl::ReaderMutexLock lock{&handle.data_->mutex_};
  if (handle.data_->executor_ == nullptr) {
    return std::optional<decltype(std::declval<Executor>().Schedule(function_object))>{};
  }

  return std::make_optional(handle.data_->executor_->Schedule(std::forward<F>(function_object)));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_EXECUTOR_H_
