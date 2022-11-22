// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarData/BaselineAndComparison.h"

#include <absl/container/flat_hash_map.h>

#include "BaselineAndComparisonHelper.h"
#include "MizarData/MizarDataProvider.h"

namespace orbit_mizar_data {

orbit_mizar_data::BaselineAndComparison CreateBaselineAndComparison(
    std::unique_ptr<MizarDataProvider> baseline, std::unique_ptr<MizarDataProvider> comparison) {
  BaselineAndComparisonHelper helper;
  auto [baseline_address_sfid, comparison_address_to_sfid, sfid_to_symbols] =
      helper.AssignSampledFunctionIds(baseline->AllAddressToFunctionSymbol(),
                                      comparison->AllAddressToFunctionSymbol());

  return {orbit_mizar_base::MakeBaseline<MizarPairedData>(std::move(baseline),
                                                          std::move(baseline_address_sfid)),
          orbit_mizar_base::MakeComparison<MizarPairedData>(std::move(comparison),
                                                            std::move(comparison_address_to_sfid)),
          std::move(sfid_to_symbols)};
}

}  // namespace orbit_mizar_data