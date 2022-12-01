// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/WhenAll.h"

#include <absl/types/span.h>

#include <algorithm>
#include <memory>

#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"

namespace orbit_base {
Future<void> WhenAll(absl::Span<const Future<void>> futures) {
  if (futures.empty()) {
    Promise<void> promise;
    promise.MarkFinished();
    return promise.GetFuture();
  }

  auto shared_state = std::make_shared<orbit_base_internal::SharedStateWhenAll<void>>();
  {
    absl::MutexLock lock{&shared_state->mutex};
    shared_state->incomplete_futures = futures.size();
  }

  for (const auto& future : futures) {
    ORBIT_CHECK(future.IsValid());
    orbit_base::FutureRegisterContinuationResult const result =
        future.RegisterContinuation([shared_state]() {
          absl::MutexLock lock{&shared_state->mutex};
          --shared_state->incomplete_futures;

          if (shared_state->incomplete_futures == 0) {
            shared_state->promise.MarkFinished();
          }
        });

    if (result != orbit_base::FutureRegisterContinuationResult::kSuccessfullyRegistered) {
      absl::MutexLock lock{&shared_state->mutex};
      --shared_state->incomplete_futures;
    }
  }

  {
    absl::MutexLock lock{&shared_state->mutex};
    if (shared_state->incomplete_futures == 0) {
      shared_state->promise.MarkFinished();
    }
  }

  return shared_state->promise.GetFuture();
}
}  // namespace orbit_base
