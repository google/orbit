// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CONSTANTS_H_
#define CLIENT_DATA_CONSTANTS_H_

#include <cstdint>

#include "ClientData/ScopeIdProvider.h"

namespace orbit_client_data {

// We use this constant to identify scopes where we don't have a corresponding Id (see
// ScopeIdProvider).
constexpr ScopeId kInvalidScopeId{0};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CONSTANTS_H_
