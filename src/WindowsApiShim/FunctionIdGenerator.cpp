// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionIdGenerator.h"

namespace orbit_windows_api_shim {

uint32_t FunctionIdGenerator::GetOrCreateFunctionIdFromKey(const std::string& function_key) {
  auto it = function_name_to_id_.find(function_key);
  if (it != function_name_to_id_.end()) return it->second;
  uint32_t new_id = next_id_++;
  function_name_to_id_.emplace(function_key, new_id);
  return new_id;
}

std::optional<uint32_t> FunctionIdGenerator::GetFunctionIdFromKey(
    const std::string& function_key) const {
  auto it = function_name_to_id_.find(function_key);
  if (it == function_name_to_id_.end()) return std::nullopt;
  return it->second;
}

}  // namespace orbit_windows_api_shim
