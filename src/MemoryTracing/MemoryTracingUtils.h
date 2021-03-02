// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_TRACING_UTILS_H_
#define MEMORY_TRACING_MEMORY_TRACING_UTILS_H_

#include "capture.pb.h"

namespace orbit_memory_tracing {

std::optional<orbit_grpc_protos::SystemMemoryUsage> ParseMemInfo(
    const std::string& meminfo_content);
std::optional<orbit_grpc_protos::SystemMemoryUsage> GetSystemMemoryUsage() noexcept;

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_TRACING_UTILS_H_