// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_TYPES_H_
#define CLIENT_DATA_CALLSTACK_TYPES_H_

#include <absl/container/flat_hash_map.h>

#include <cstdint>

#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

using ThreadID = uint32_t;

[[nodiscard]] std::string CallstackTypeToString(
    orbit_client_protos::CallstackInfo::CallstackType callstack_type);

[[nodiscard]] std::string CallstackTypeToDescription(
    orbit_client_protos::CallstackInfo::CallstackType callstack_type);

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_TYPES_H_
