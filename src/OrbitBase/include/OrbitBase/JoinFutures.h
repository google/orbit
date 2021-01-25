// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_JOIN_FUTURES_H_
#define ORBIT_BASE_JOIN_FUTURES_H_

#include "OrbitBase/Future.h"
#include "absl/types/span.h"

namespace orbit_base {

// Returns a future which completes when all futures in the argument have completed.
// Currently only available for Future<void>.
[[nodiscard]] Future<void> JoinFutures(absl::Span<const Future<void>> futures);
}  // namespace orbit_base

#endif  // ORBIT_BASE_JOIN_FUTURES_H_