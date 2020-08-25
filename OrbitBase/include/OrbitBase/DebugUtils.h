// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_DEBUG_UTILS_H_
#define ORBIT_BASE_DEBUG_UTILS_H_

#include <string>

#include "Logging.h"
#include "OrbitBase/Profiling.h"

#define DEBUG_UTILS_ENABLED 0  // Enable locally only.
#if DEBUG_UTILS_ENABLED

// Print variable's name and value in the form "name = value".
#define PRINT_VAR(x) LOG("%s = %s", #x, std::to_string(x))
// Print current function's name, file and line as url, and thread id.
#define PRINT_FUNC \
  LOG("%s %s(%u) %s", __FUNCTION__, __FILE__, __LINE__, std::to_string(GetCurrentThreadId()))

#endif  // DEBUG_UTILS_ENABLED

#endif  // ORBIT_BASE_DEBUG_UTILS_H_
