// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef API_UTILS_GET_FUNCTION_TABLE_ADDRESS_PREFIX_H_
#define API_UTILS_GET_FUNCTION_TABLE_ADDRESS_PREFIX_H_

#include <string>

namespace orbit_api_utils {

// These are the possible prefixes, depending on the platform, of the function declared in Orbit.h
// that we need to call in the tracee after having loaded liborbit.so into it.

// This is the prefix on Linux.
static const std::string kOrbitApiGetFunctionTableAddressPrefix{
    "orbit_api_get_function_table_address_v"};

// This is the prefix on Windows.
static const std::string kOrbitApiGetFunctionTableAddressWinPrefix{
    "orbit_api_get_function_table_address_win_v"};

}  // namespace orbit_api_utils

#endif  // API_UTILS_GET_FUNCTION_TABLE_ADDRESS_PREFIX_H_
