// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BaselineAndComparisonHelper.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <string>

namespace orbit_mizar_data {

template <typename K, typename V>
[[nodiscard]] static absl::flat_hash_set<V> ValueSet(const absl::flat_hash_map<K, V>& map) {
  absl::flat_hash_set<V> result;
  std::transform(std::begin(map), std::end(map), std::inserter(result, std::begin(result)),
                 [](const std::pair<K, V> pair) { return pair.second; });
  return result;
}

static absl::flat_hash_map<uint64_t, SFID> AddressToSFID(
    const absl::flat_hash_map<uint64_t, std::string>& address_to_name,
    const absl::flat_hash_map<std::string, SFID>& name_to_sfid) {
  absl::flat_hash_map<uint64_t, SFID> address_to_sfid;
  for (const auto& [address, name] : address_to_name) {
    if (const auto it = name_to_sfid.find(name); it != name_to_sfid.end()) {
      address_to_sfid.try_emplace(address, it->second);
    }
  }
  return address_to_sfid;
}

[[nodiscard]] AddressToIdAndIdToName AssignSampledFunctionIds(
    const absl::flat_hash_map<uint64_t, std::string>& baseline_address_to_name,
    const absl::flat_hash_map<uint64_t, std::string>& comparison_address_to_name) {
  absl::flat_hash_set<std::string> baseline_names =
      ValueSet<uint64_t, std::string>(baseline_address_to_name);
  absl::flat_hash_set<std::string> comparison_names =
      ValueSet<uint64_t, std::string>(comparison_address_to_name);

  absl::flat_hash_map<std::string, SFID> name_to_sfid;
  absl::flat_hash_map<SFID, std::string> sfid_to_name;

  uint64_t next_sfid_value = 1;
  for (const std::string& name : baseline_names) {
    if (comparison_names.contains(name) && !name_to_sfid.contains(name)) {
      name_to_sfid.try_emplace(name, SFID(next_sfid_value));
      sfid_to_name.try_emplace(SFID(next_sfid_value), name);
      next_sfid_value++;
    }
  }

  absl::flat_hash_map<uint64_t, SFID> baseline_address_to_sfid =
      AddressToSFID(baseline_address_to_name, name_to_sfid);
  absl::flat_hash_map<uint64_t, SFID> comparison_address_to_sfid =
      AddressToSFID(comparison_address_to_name, name_to_sfid);

  return {std::move(baseline_address_to_sfid), std::move(comparison_address_to_sfid),
          std::move(sfid_to_name)};
}

}  // namespace orbit_mizar_data