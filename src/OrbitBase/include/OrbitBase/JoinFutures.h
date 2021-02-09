// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_JOIN_FUTURES_H_
#define ORBIT_BASE_JOIN_FUTURES_H_

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/Promise.h"
#include "absl/types/span.h"

namespace orbit_base_internal {

template <typename T>
class SharedStateJoin {
  orbit_base::Promise<std::vector<T>> promise;
  absl::Mutex mutex;
  size_t incomplete_futures;
  std::vector<std::optional<T>> results;

  void SetResultsInPromise() {
    std::vector<T> output;
    output.reserve(results.size());
    std::transform(results.begin(), results.end(), std::back_inserter(output),
                   [](std::optional<T>& element) {
                     CHECK(element.has_value());
                     return *std::move(element);
                   });
    results.clear();
    promise.SetResult(std::move(output));
  }

 public:
  void SetNumberOfFutures(size_t size) {
    absl::MutexLock lock{&mutex};
    incomplete_futures = size;
    results.resize(size);
  }

  void SetResult(size_t index, const T& argument) {
    absl::MutexLock lock{&mutex};
    if (!results[index].has_value()) {
      results[index] = argument;
      --incomplete_futures;
    }

    if (incomplete_futures == 0) {
      SetResultsInPromise();
    }
  }

  orbit_base::Future<std::vector<T>> GetFuture() const { return promise.GetFuture(); }
};

template <>
struct SharedStateJoin<void> {
  orbit_base::Promise<void> promise;
  absl::Mutex mutex;
  size_t incomplete_futures;
};
}  // namespace orbit_base_internal

namespace orbit_base {

// Returns a future which completes when all futures in the argument have completed.
// Currently only available for Future<void>.
[[nodiscard]] Future<void> JoinFutures(absl::Span<const Future<void>> futures);

template <typename T>
Future<std::vector<T>> JoinFutures(absl::Span<const Future<T>> futures) {
  if (futures.empty()) {
    Promise<std::vector<T>> promise;
    promise.SetResult({});
    return promise.GetFuture();
  }

  auto shared_state = std::make_shared<orbit_base_internal::SharedStateJoin<T>>();
  shared_state->SetNumberOfFutures(futures.size());

  for (auto it = futures.begin(); it != futures.end(); ++it) {
    const auto& future = *it;
    CHECK(future.IsValid());

    const size_t index = it - futures.begin();
    RegisterContinuationOrCallDirectly(future, [shared_state, index](const T& argument) {
      shared_state->SetResult(index, argument);
    });
  }

  return shared_state->GetFuture();
}
}  // namespace orbit_base

#endif  // ORBIT_BASE_JOIN_FUTURES_H_