// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_LIST_MODULES_ETW_H_
#define WINDOWS_TRACING_LIST_MODULES_ETW_H_

#include "WindowsUtils/ListModules.h"

namespace orbit_windows_tracing {

// List all modules of the process identified by "pid" using ETW.
[[nodiscard]] std::vector<orbit_windows_utils::Module> ListModulesEtw(uint32_t pid);

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_LIST_MODULES_H_
