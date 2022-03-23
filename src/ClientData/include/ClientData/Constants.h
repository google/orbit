// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CONSTANTS_H_
#define CLIENT_DATA_CONSTANTS_H_

#include <cstdint>
namespace orbit_client_data {

// The value returned by `AppInterface::ProvideScopeId` if the supplied
// `orbit_client_protos::TimerInfo` is neither of type kApiScope, kApiScopeAsync nor refers to a
// dynamic instrumentation event.
constexpr uint64_t kInvalidScopeId = 0;

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CONSTANTS_H_
