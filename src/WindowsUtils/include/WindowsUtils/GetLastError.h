// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_GET_LAST_ERROR_H_
#define WINDOWS_UTILS_GET_LAST_ERROR_H_

#include <string>

namespace orbit_windows_utils {

[[nodiscard]] std::string GetLastErrorAsString();

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_GET_LAST_ERROR_H_
