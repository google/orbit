// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_UTILS_API_ENABLE_INFO_H_
#define ORBIT_API_UTILS_API_ENABLE_INFO_H_

#include <stdint.h>

#include <type_traits>

namespace orbit_api {

// This structure is used on Windows when calling "orbit_api_set_enabled_from_struct" remotely
// using the "CreateRemoteThread" api, which takes in a single parameter for the thread function.
// This struct needs to be POD so that we can easily copy it in a remote process address space.
struct ApiEnableInfo {
  // Address of orbit_api_get_function_table_address_win_vN function.
  uint64_t orbit_api_function_address;
  uint64_t api_version;
  bool api_enabled;
};

static_assert(std::is_trivial<ApiEnableInfo>::value, "ApiEnableInfo must be a trivial type.");

}  // namespace orbit_api

#endif  // ORBIT_API_UTILS_API_ENABLE_INFO_H_
