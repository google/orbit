// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_FUTURE_H_
#define ORBIT_BASE_FUTURE_H_

#include <absl/synchronization/mutex.h>

#include <future>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SharedState.h"

namespace orbit_base_internal {

// For befriending Promise we need a forward declaration.
template <typename T>
class PromiseBase;

// FutureBase<T> is an internal base class for Future<T>. Check out Future<T> below for more
// information.
template <typename T>
class FutureBase {
 public:
  FutureBase(const FutureBase&) = default;
  FutureBase& operator=(const FutureBase&) = default;
  FutureBase(FutureBase&&) = default;
  FutureBase& operator=(FutureBase&&) = default;
  ~FutureBase() noexcept = default;

  [[nodiscard]] bool IsValid() const { return shared_state_.use_count() > 0; }

 protected:
  explicit FutureBase(std::shared_ptr<SharedState<T>> shared_state)
      : shared_state_{std::move(shared_state)} {}

  std::shared_ptr<SharedState<T>> shared_state_;
};

}  // namespace orbit_base_internal

namespace orbit_base {

// A future type, similar to std::future but prepared to integrate with
// Orbit-specific code like MainThreadExecutor and ThreadPool.
//
// Future is a result type for an asynchronous task, where the result
// will not be available right away, but to a later point in time.
//
// A valid Future<T> is created from a Promise<T>. The latter lives in the
// asynchronous task and its purpose is to notify the Future<T> when the result
// is available.
//
// Use Future<T>::IsValid() to check if a future is connected to a promise or holds
// a result value.
//
// Call Future<T>::IsFinished() to figure out if the result is already available.
//
// To retrieve the T, you can call Future<T>::Get(). It will block when the result
// is not yet available.
//
// Real-world examples should involved an executor like MainThreadExecutor or ThreadPool.
// Check-out their tests and docs to learn how to use Future.
template <typename T>
class [[nodiscard]] Future : public orbit_base_internal::FutureBase<T> {
 public:
  [[nodiscard]] bool IsFinished() const {
    if (this->shared_state_.use_count() == 0) return false;

    absl::MutexLock lock{&this->shared_state_->mutex};
    return this->shared_state_->result.has_value();
  }

  const T& Get() const {
    Wait();

    absl::MutexLock lock{&this->shared_state_->mutex};
    return this->shared_state_->result.value();
  }

  // Consider this an internal method which only brings functionality to
  // properly designed waiting code like orbit_qt_utils::FutureWatcher.

  // The continuation is potentially executed on a background thread,
  // which means you have to be aware of race-conditions while registering
  // the continuation and potential mutex deadlocks in the continuation.
  void RegisterContinuation(std::function<void(const T&)> continuation) const {
    if (!this->IsValid()) return;

    absl::MutexLock lock{&this->shared_state_->mutex};
    this->shared_state_->continuations.emplace_back(std::move(continuation));
  }

  void Wait() const {
    CHECK(this->IsValid());
    absl::MutexLock lock{&this->shared_state_->mutex};
    this->shared_state_->mutex.Await(absl::Condition(
        +[](std::optional<T>* result) { return result->has_value(); },
        &this->shared_state_->result));
  }

 private:
  friend orbit_base_internal::PromiseBase<T>;

  using orbit_base_internal::FutureBase<T>::FutureBase;
};

// Future<void> is a specialization of Future<T> for asynchronous tasks that return `void`.
//
// In this case the future won't be able to transer any return type to the caller, but it can
// notify the caller, when the asynchronous tasks completes.
//
// Unlike Future<T>, Future<void> has no `Get()` method since there is no return value.
template <>
class Future<void> : public orbit_base_internal::FutureBase<void> {
 public:
  [[nodiscard]] bool IsFinished() const {
    if (shared_state_.use_count() == 0) return false;

    absl::MutexLock lock{&shared_state_->mutex};
    return shared_state_->finished;
  }

  // Check Future<T>::RegisterContinuation for a warning about this function.
  // TLDR: You probably want to use `Future<void>::Then` instead!
  void RegisterContinuation(std::function<void()> continuation) const {
    if (!IsValid()) return;

    absl::MutexLock lock{&shared_state_->mutex};
    shared_state_->continuations.emplace_back(std::move(continuation));
  }

  void Wait() const {
    CHECK(IsValid());
    absl::MutexLock lock{&this->shared_state_->mutex};
    shared_state_->mutex.Await(absl::Condition(
        +[](bool* finished) { return *finished; }, &shared_state_->finished));
  }

 private:
  friend orbit_base_internal::PromiseBase<void>;

  using orbit_base_internal::FutureBase<void>::FutureBase;
};
}  // namespace orbit_base

#endif  // ORBIT_BASE_FUTURE_H_