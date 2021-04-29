// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_ENABLE_IN_TRACEE_H_
#define ORBIT_API_ENABLE_IN_TRACEE_H_

#include "OrbitBase/Result.h"
#include "capture.pb.h"

namespace orbit_api {

ErrorMessageOr<void> EnableApiInTracee(const orbit_grpc_protos::CaptureOptions& capture_options);
ErrorMessageOr<void> DisableApiInTracee(const orbit_grpc_protos::CaptureOptions& capture_options);

}  // namespace orbit_api

#endif
