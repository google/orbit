// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_WHEN_ANY_H_
#define ORBIT_BASE_WHEN_ANY_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <variant>

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/VoidToMonostate.h"

namespace orbit_base_internal {

template <typename... Ts>
class SharedStateWhenAny {
 public:
  template <size_t index>
  void SetResult(const typename std::variant_alternative_t<index, std::variant<Ts...>>& argument)
      ABSL_LOCKS_EXCLUDED(mutex) {
    absl::MutexLock lock{&mutex};
    if (promise.HasResult()) return;

    promise.SetResult(std::variant<Ts...>{std::in_place_index_t<index>{}, argument});
  }

  [[nodiscard]] orbit_base::Future<std::variant<Ts...>> GetFuture() const {
    absl::MutexLock lock{&mutex};
    return promise.GetFuture();
  }

 private:
  orbit_base::Promise<std::variant<Ts...>> promise ABSL_GUARDED_BY(mutex);
  mutable absl::Mutex mutex;
};

template <typename... Args, std::size_t... Indices>
orbit_base::Future<std::variant<orbit_base::VoidToMonostate_t<Args>...>> WhenAnyImpl(
    orbit_base::Future<Args>... futures, std::index_sequence<Indices...> /* indexes */) {
  ORBIT_CHECK(futures.IsValid() && ...);

  auto shared_state = std::make_shared<
      orbit_base_internal::SharedStateWhenAny<orbit_base::VoidToMonostate_t<Args>...>>();

  (orbit_base::RegisterContinuationOrCallDirectly(
       futures,
       // Having the lambda expression taking a parameter pack as its arguments allows us to express
       // 0 arguments (void) and 1 argument (any other T) in a single continuation.
       [shared_state](const auto&... arguments) {
         if constexpr (orbit_base::IsMonostate<Indices, Args...>::value) {
           shared_state->template SetResult<Indices>(std::monostate{});
         } else {
           // `arguments` is a parameter pack of at most 1 argument. In this if-constexpr-branch we
           // already know it's exactly one argument, so `(arguments, ...)` just unfolds to a single
           // argument.
           shared_state->template SetResult<Indices>((arguments, ...));
         }
       }),
   ...);

  return shared_state->GetFuture();
}
}  // namespace orbit_base_internal

namespace orbit_base {

// Returns a future which completes when any of the given futures has completed.
// The returning future will contain a variant that holds the value of the completed future.
// Note that a future of type `Future<void>` will be represented as `std::monostate` in the variant.
template <typename Arg0, typename... Args>
[[nodiscard]] Future<std::variant<VoidToMonostate_t<Arg0>, VoidToMonostate_t<Args>...>> WhenAny(
    Future<Arg0> future0, Future<Args>... futures) {
  return orbit_base_internal::WhenAnyImpl<Arg0, Args...>(
      std::move(future0), std::move(futures)...,
      std::make_index_sequence<1 + sizeof...(futures)>());
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_WHEN_ANY_H_