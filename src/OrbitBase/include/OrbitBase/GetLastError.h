// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_GET_LAST_ERROR_H_
#define ORBIT_BASE_GET_LAST_ERROR_H_

#include <string>

namespace orbit_base {

[[nodiscard]] std::string GetLastErrorAsString();

}  // namespace orbit_base

#endif  // ORBIT_BASE_GET_LAST_ERROR_H_
