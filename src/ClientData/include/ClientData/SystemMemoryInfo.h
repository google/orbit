// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_SYSTEM_MEMORY_INFO_H_
#define CLIENT_DATA_SYSTEM_MEMORY_INFO_H_

#include <cstdint>

#include "GrpcProtos/Constants.h"

namespace orbit_client_data {

struct SystemMemoryInfo {
  uint64_t timestamp_ns;
  int64_t total_kb;
  int64_t free_kb;
  int64_t available_kb;
  int64_t buffers_kb;
  int64_t cached_kb;

  [[nodiscard]] bool HasMissingInfo() const {
    return total_kb == orbit_grpc_protos::kMissingInfo ||
           free_kb == orbit_grpc_protos::kMissingInfo ||
           buffers_kb == orbit_grpc_protos::kMissingInfo ||
           cached_kb == orbit_grpc_protos::kMissingInfo;
  }
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_SYSTEM_MEMORY_INFO_H_
