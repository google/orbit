// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_WINDOWS_API_SHIM_FUNCTION_ID_GENERATOR_H_
#define ORBIT_WINDOWS_API_SHIM_FUNCTION_ID_GENERATOR_H_

#include <absl/container/flat_hash_map.h>

#include <string_view>

namespace orbit_windows_api_shim {

// FunctionIdGenerator assigns a unique id to a module-function pair (function_key).
// This class is not thread-safe.
class FunctionIdGenerator {
 public:
  FunctionIdGenerator() = default;
  uint32_t GetOrCreateFunctionIdFromKey(const std::string& function_key);
  std::optional<uint32_t> GetFunctionIdFromKey(const std::string& function_key) const;

 private:
  absl::flat_hash_map<std::string, uint32_t> function_name_to_id_;
  uint32_t next_id_ = 0;
};

}  // namespace orbit_windows_api_shim

#endif  // ORBIT_WINDOWS_API_SHIM_FUNCTION_ID_GENERATOR_H_
