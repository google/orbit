// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_WINDOWS_TRACING_UTILS_H_
#define WINDOWS_TRACING_WINDOWS_TRACING_UTILS_H_

#include <vector>

#include "capture.pb.h"
#include "process.pb.h"

namespace orbit_windows_tracing {

// List all currently running processes.
[[nodiscard]] std::vector<orbit_grpc_protos::ProcessInfo> ListProcesses();

// List all modules of the process identified by "pid".
[[nodiscard]] std::vector<orbit_grpc_protos::ModuleInfo> ListModules(uint32_t pid);

// List all currently running threads of the process identified by "pid".
[[nodiscard]] std::vector<orbit_grpc_protos::ThreadName> ListThreads(uint32_t pid);

// List all threads.
[[nodiscard]] std::vector<orbit_grpc_protos::ThreadName> ListAllThreads();

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_WINDOWS_TRACING_UTILS_H_
