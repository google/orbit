// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_ADJUST_TOKEN_PRIVILEGE_H_
#define WINDOWS_UTILS_ADJUST_TOKEN_PRIVILEGE_H_

#include <Windows.h>

#include "OrbitBase/Logging.h"

namespace orbit_windows_utils {

// Enables or disables privileges in the specified access token.
// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
ErrorMessageOr<void> AdjustTokenPrivilege(LPCTSTR token_name, bool enabled);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_ADJUST_TOKEN_PRIVILEGE_H_