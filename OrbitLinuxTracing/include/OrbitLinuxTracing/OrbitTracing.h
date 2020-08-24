// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_ORBIT_TRACING_H_
#define ORBIT_LINUX_TRACING_ORBIT_TRACING_H_

#include <OrbitBase/Tracing.h>

#include "capture.pb.h"

namespace LinuxTracing {

orbit_grpc_protos::FunctionCall FunctionCallFromTracingScope(orbit::tracing::Scope);

}

#endif  // ORBIT_LINUX_TRACING_ORBIT_TRACING_H_
