// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_HELPER_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_HELPER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <string>

namespace orbit_mizar_data {

struct AddressToIdAndIdToName {
  absl::flat_hash_map<uint64_t, uint64_t> baseline_address_to_sampled_function_id;
  absl::flat_hash_map<uint64_t, uint64_t> comparison_address_to_sampled_function_id;
  absl::flat_hash_map<uint64_t, std::string> frame_id_to_name;
};

[[nodiscard]] AddressToIdAndIdToName AssignSampledFunctionIds(
    const absl::flat_hash_map<uint64_t, std::string>& baseline_address_to_name,
    const absl::flat_hash_map<uint64_t, std::string>& comparison_address_to_name);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_HELPER_H_
