// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TRACEPOINT_CUSTOM_H_
#define CLIENT_DATA_TRACEPOINT_CUSTOM_H_

#include <absl/container/flat_hash_set.h>

#include "GrpcProtos/tracepoint.pb.h"

namespace orbit_client_data {

namespace internal {
struct HashTracepointInfo {
  size_t operator()(const orbit_grpc_protos::TracepointInfo& info) const {
    return std::hash<std::string>{}(info.category()) * 37 + std::hash<std::string>{}(info.name());
  }
};

struct EqualTracepointInfo {
  bool operator()(const orbit_grpc_protos::TracepointInfo& left,
                  const orbit_grpc_protos::TracepointInfo& right) const {
    return left.category().compare(right.category()) == 0 && left.name().compare(right.name()) == 0;
  }
};

}  // namespace internal

using TracepointInfoSet =
    absl::flat_hash_set<orbit_grpc_protos::TracepointInfo, internal::HashTracepointInfo,
                        internal::EqualTracepointInfo>;

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TRACEPOINT_CUSTOM_H_