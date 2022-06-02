// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BaselineAndComparisonHelper.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

namespace orbit_mizar_data {

template <typename K, typename V>
[[nodiscard]] static absl::flat_hash_set<V> ValueSet(const absl::flat_hash_map<K, V>& map) {
  absl::flat_hash_set<V> result;
  std::transform(std::begin(map), std::end(map), std::inserter(result, std::begin(result)),
                 [](const std::pair<K, V> pair) { return pair.second; });
  return result;
}

static absl::flat_hash_map<uint64_t, uint64_t> AddressToSampledFunctionId(
    const absl::flat_hash_map<uint64_t, std::string>& address_to_name,
    const absl::flat_hash_map<std::string, uint64_t>& name_to_frame_id) {
  absl::flat_hash_map<uint64_t, uint64_t> address_to_frame_id;
  for (const auto& [address, name] : address_to_name) {
    if (const auto it = name_to_frame_id.find(name); it != name_to_frame_id.end()) {
      address_to_frame_id[address] = it->second;
    }
  }
  return address_to_frame_id;
}

[[nodiscard]] std::tuple<absl::flat_hash_map<uint64_t, uint64_t>,
                         absl::flat_hash_map<uint64_t, uint64_t>,
                         absl::flat_hash_map<uint64_t, std::string>>
AssignSampledFunctionIds(
    const absl::flat_hash_map<uint64_t, std::string>& baseline_address_to_name,
    const absl::flat_hash_map<uint64_t, std::string>& comparison_address_to_name) {
  absl::flat_hash_set<std::string> baseline_names = ValueSet(baseline_address_to_name);
  absl::flat_hash_set<std::string> comparison_names = ValueSet(comparison_address_to_name);

  absl::flat_hash_map<std::string, uint64_t> name_to_frame_id;
  absl::flat_hash_map<uint64_t, std::string> frame_id_to_name;

  uint64_t next_frame_id = 1;
  for (const std::string& name : baseline_names) {
    if (comparison_names.contains(name) && !name_to_frame_id.contains(name)) {
      name_to_frame_id[name] = next_frame_id;
      frame_id_to_name[next_frame_id] = name;
      next_frame_id++;
    }
  }

  absl::flat_hash_map<uint64_t, uint64_t> baseline_address_to_sampled_function_id =
      AddressToSampledFunctionId(baseline_address_to_name, name_to_frame_id);
  absl::flat_hash_map<uint64_t, uint64_t> comparison_address_to_sampled_function_id =
      AddressToSampledFunctionId(comparison_address_to_name, name_to_frame_id);

  return std::make_tuple(baseline_address_to_sampled_function_id,
                         comparison_address_to_sampled_function_id, frame_id_to_name);
}

}  // namespace orbit_mizar_data