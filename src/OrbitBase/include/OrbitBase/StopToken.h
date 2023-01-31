// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_TOKEN_H_
#define ORBIT_BASE_STOP_TOKEN_H_

#include <memory>
#include <variant>

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/WhenAny.h"

namespace orbit_base {

// StopToken together with StopSource is designed to enable thread safe task cancellation. Create a
// StopToken from StopSource::GetStopToken and it into a thread on creation. Then check periodically
// whether StopToken::IsStopRequested, to see if a stop has been requested by the corresponding
// StopSource.
class StopToken {
  friend class StopSource;

 public:
  // Returns true if StopToken has a stop-state, otherwise false. If StopToken has a stop-state and
  // a stop has already been requested, this function still returns true.
  [[nodiscard]] bool IsStopPossible() const { return future_.IsValid(); }
  [[nodiscard]] bool IsStopRequested() const {
    ORBIT_CHECK(IsStopPossible());
    return future_.IsFinished();
  }

  [[nodiscard]] Future<void> GetFuture() const { return future_; }

 private:
  explicit StopToken(Future<void> future) : future_{std::move(future)} {}

  orbit_base::Future<void> future_;
};

// Takes a `Future<T>` and a `StopToken` and returns a `Future<CanceledOr<T>>`. The returned future
// will complete when either the input future completes or when the StopToken indicates that a stop
// was requested - whatever happens first. When a stop was requested the resulting value will be
// marked as `Canceled`, otherwise it will contain the `T`.
//
// Note that this doesn't mean that the operation that was supposed to produce the result of
// `Future<T>` will be interrupted. It will finish as usual, but the result will not be used
// by this code path.
// To make a background job truly cancelable you have to make the task take a StopToken instead.
//
// This function is still useful though to make an uninterruptable task "cancelable".
template <typename T>
Future<CanceledOr<T>> WhenValueOrCanceled(const Future<T>& value, const StopToken& stop_token) {
  orbit_base::ImmediateExecutor executor{};
  return orbit_base::WhenAny(value, stop_token.GetFuture())
      .Then(&executor,
            [](const std::variant<orbit_base::VoidToMonostate_t<T>, std::monostate>& result)
                -> CanceledOr<T> {
              // `WhenAny` returns a future to a variant. This variant will hold the first
              // alternative (index 0) when the future `value` completed or the second alternative
              // (index 1) when the `stop_token` indicated a cancelation. So we return `Canceled` if
              // the index is 1:
              if (result.index() == 1) return outcome::failure(Canceled{});
              if constexpr (std::is_same_v<T, void>) {
                return outcome::success();
              } else {
                return outcome::success(std::get<0>(result));
              }
            });
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_STOP_TOKEN_H_