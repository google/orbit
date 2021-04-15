// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Api/InitializeInTracee.h"

#include "Api/Orbit.h"
#include "OrbitBase/Result.h"

using orbit_grpc_protos::CaptureOptions;

namespace orbit_api {

ErrorMessageOr<void> InitializeInTracee(const CaptureOptions& /*capture_options*/) {
  // Dummy implementation until "ExecuteInProcess" functionality makes it into main.
  return outcome::success();
}

}  // namespace orbit_api
