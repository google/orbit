// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROMISE_H_
#define ORBIT_BASE_PROMISE_H_

#include <absl/synchronization/mutex.h>

#include <memory>
#include <type_traits>

#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SharedState.h"

namespace orbit_base_internal {

// PromiseBase is an internal base class of Promise<T>. Check out Promise<T> below for more
// information.
template <typename T>
class PromiseBase {
 public:
  explicit PromiseBase() : shared_state_{std::make_shared<SharedState<T>>()} {}
  PromiseBase(const PromiseBase&) = delete;
  PromiseBase& operator=(const PromiseBase&) = delete;
  PromiseBase(PromiseBase&&) = default;
  PromiseBase& operator=(PromiseBase&&) = default;
  ~PromiseBase() = default;

  [[nodiscard]] orbit_base::Future<T> GetFuture() const {
    return orbit_base::Future<T>{shared_state_};
  }
  [[nodiscard]] bool IsValid() const { return shared_state_.use_count() > 0; }

 protected:
  std::shared_ptr<SharedState<T>> shared_state_;
};

}  // namespace orbit_base_internal

namespace orbit_base {

// Promise<T> is a promise type, similar to std::promise<T> which can store a
// value to be later (asynchrnously) recalled by the corresponding Future<T>.
//
// Use Promise<T>::GetFuture to get the corresponding future.
//
// You probably only want to use this type when you write an executor. For
// most occasions the executors ThreadPool and MainThreadExecutor fulfill the
// general need.
//
// Example:
// Promise<int> promise{};
//
// auto future = promise.GetFuture();
// /* pass the future to the callee */
//
// int result = do_work();
// promise.SetResult(result);
template <typename T>
class Promise : public orbit_base_internal::PromiseBase<T> {
  static_assert(!std::is_reference_v<T>, "The promise's value type may not be a reference!");
  static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>,
                "The promise's value type needs to be at least move-constructible!");

 public:
  using orbit_base_internal::PromiseBase<T>::PromiseBase;

  void SetResult(T result) {
    absl::MutexLock lock{&this->shared_state_->mutex};

    for (auto& continuation : this->shared_state_->continuations) {
      continuation(result);
    }

    this->shared_state_->result.emplace(std::move(result));
  }

  [[nodiscard]] bool HasResult() const {
    if (!this->IsValid()) return false;
    absl::MutexLock lock{&this->shared_state_->mutex};
    return this->shared_state_->result.has_value();
  }
};

template <>
class Promise<void> : public orbit_base_internal::PromiseBase<void> {
 public:
  using PromiseBase<void>::PromiseBase;

  void MarkFinished() {
    absl::MutexLock lock{&this->shared_state_->mutex};

    for (auto& continuation : this->shared_state_->continuations) {
      continuation();
    }

    this->shared_state_->finished = true;
  }

  [[nodiscard]] bool IsFinished() const {
    if (!IsValid()) return false;
    absl::MutexLock lock{&this->shared_state_->mutex};
    return this->shared_state_->finished;
  }
};
}  // namespace orbit_base

#endif  // ORBIT_BASE_PROMISE_H_