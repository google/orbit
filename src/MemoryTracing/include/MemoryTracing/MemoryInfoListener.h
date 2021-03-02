// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_INFO_LISTENER_H_
#define MEMORY_TRACING_MEMORY_INFO_LISTENER_H_

#include "capture.pb.h"

namespace orbit_memory_tracing {

// This class serves as an event listener for the SystemMemoryUsage event.
class MemoryInfoListener {
 public:
  virtual ~MemoryInfoListener() = default;
  virtual void OnSystemMemoryUsage(orbit_grpc_protos::SystemMemoryUsage memory_info) = 0;
};

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_INFO_LISTENER_H_