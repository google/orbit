// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarData/BaselineAndComparison.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <algorithm>
#include <iterator>
#include <string>

#include "BaselineAndComparisonHelper.h"
#include "ClientData/CaptureData.h"
#include "MizarData/MizarData.h"

namespace orbit_mizar_data {

orbit_mizar_data::BaselineAndComparison CreateBaselineAndComparison(
    std::unique_ptr<MizarDataProvider> baseline, std::unique_ptr<MizarDataProvider> comparison) {
  const absl::flat_hash_map<uint64_t, std::string> baseline_address_to_name =
      baseline->AllAddressToName();
  const absl::flat_hash_map<uint64_t, std::string> comparison_address_to_name =
      comparison->AllAddressToName();

  auto [baseline_address_to_frame_id, comparison_address_to_frame_id, frame_id_to_name] =
      AssignSampledFunctionIds(baseline_address_to_name, comparison_address_to_name);

  return {{std::move(baseline), std::move(baseline_address_to_frame_id)},
          {std::move(comparison), std::move(comparison_address_to_frame_id)},
          std::move(frame_id_to_name)};
}

}  // namespace orbit_mizar_data