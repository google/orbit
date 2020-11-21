// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PROFILING_H_
#define ORBIT_BASE_PROFILING_H_

#ifdef _WIN32
#include "Platform/Windows/Profiling.h"
#else
#include "Platform/Linux/Profiling.h"
#endif

#endif  // ORBIT_BASE_PROFILING_H_
