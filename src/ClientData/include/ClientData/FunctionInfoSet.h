// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_FUNCTION_INFO_SET_H_
#define CLIENT_DATA_FUNCTION_INFO_SET_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"

namespace orbit_client_data {

namespace internal {
struct HashFunctionInfo {
  size_t operator()(const orbit_client_protos::FunctionInfo& function) const {
    return (std::hash<uint64_t>{}(function.address()) * 37 +
            std::hash<std::string>{}(function.module_build_id())) *
               37 +
           std::hash<std::string>{}(function.module_path());
  }
};

// Compare functions by module path and address
struct EqualFunctionInfo {
  bool operator()(const orbit_client_protos::FunctionInfo& left,
                  const orbit_client_protos::FunctionInfo& right) const {
    return left.module_path() == right.module_path() &&
           left.module_build_id() == right.module_build_id() && left.address() == right.address();
  }
};

}  // namespace internal

using FunctionInfoSet =
    absl::flat_hash_set<orbit_client_protos::FunctionInfo, internal::HashFunctionInfo,
                        internal::EqualFunctionInfo>;

template <class V>
using FunctionInfoMap =
    absl::flat_hash_map<orbit_client_protos::FunctionInfo, V, internal::HashFunctionInfo,
                        internal::EqualFunctionInfo>;

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_FUNCTION_INFO_SET_H_
