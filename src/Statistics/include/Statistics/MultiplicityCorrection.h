// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_MULTIPLICITY_CORRECTION_H_
#define STATISTICS_MULTIPLICITY_CORRECTION_H_

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

namespace orbit_statistics {

// The simplest correction known in the literature. Very easy to reason about. Shouldn't be used but
// for testing or for lack of a better alternative.
template <typename K>
[[nodiscard]] absl::flat_hash_map<K, double> BonferroniCorrection(
    const absl::flat_hash_map<K, double>& pvalues) {
  absl::flat_hash_map<K, double> corrected;
  std::transform(
      std::begin(pvalues), std::end(pvalues), std::inserter(corrected, std::begin(corrected)),
      [&pvalues](const auto& key_to_pvalue) {
        return std::make_pair(key_to_pvalue.first, key_to_pvalue.second * pvalues.size());
      });
  return corrected;
}

// A practical correction (unlike Bonferroni).
template <typename K>
[[nodiscard]] absl::flat_hash_map<K, double> HolmBonferroniCorrection(
    const absl::flat_hash_map<K, double>& pvalues) {
  std::vector<std::pair<K, double>> corrected_pvalues(std::begin(pvalues), std::end(pvalues));
  std::sort(std::begin(corrected_pvalues), std::end(corrected_pvalues),
            [](const auto& a, const auto& b) { return a.second < b.second; });

  size_t correcting_multiplier = pvalues.size();
  double max_corrected_pvalue = 0.0;
  for (auto& [ignored, pvalue] : corrected_pvalues) {
    pvalue = std::max(max_corrected_pvalue, pvalue * correcting_multiplier);
    pvalue = std::min(pvalue, 1.0);
    max_corrected_pvalue = pvalue;
    --correcting_multiplier;
  };
  return {std::begin(corrected_pvalues), std::end(corrected_pvalues)};
}

}  // namespace orbit_statistics

#endif  // STATISTICS_MULTIPLICITY_CORRECTION_H_
