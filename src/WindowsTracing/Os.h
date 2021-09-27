// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_OS_H_
#define WINDOWS_TRACING_OS_H_

#include <Windows.h>

#include "OrbitBase/Logging.h"

namespace orbit_windows_tracing {

ErrorMessageOr<void> AdjustTokenPrivilege(LPCTSTR token_name, bool enabled);

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_OS_H_