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

#include "OrbitBase/Sort.h"

namespace orbit_statistics {

// Here we implement multiplicity correction methods (a term from statistics).
// TL;DR. An individual statistical test yields a single pvalue. That pvalue can be compared against
// the user-defined significance level alpha (e.g. alpha=0.05), and raise an alarm if
// `pvalue < alpha`, thus controlling the probability of false alarm -- it will be around alpha.
// Now, consider a case where a series of statistical tests takes place (e.g. 1000 of them). And we
// don't want to see `~1000*alpha` false alarms. We rather wish to keep the probability of _any_
// positive number of false alarms under alpha. That is, we wish to control Family-wise error rate.
// Multiplicity correction yields corrected pvalues. One can compare the corrected pvalues against
// alpha in the same manner as it is done for pvalues. The chance of _at least one_ false alarm will
// be around alpha.

// The simplest correction known in the literature. Very easy to reason about. Shouldn't be used
// but for testing or for lack of a better alternative.
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
  orbit_base::sort(std::begin(corrected_pvalues), std::end(corrected_pvalues),
                   &std::pair<K, double>::second);

  size_t correcting_multiplier = pvalues.size();
  double max_corrected_pvalue = 0.0;
  for (auto& [unused_key, pvalue] : corrected_pvalues) {
    pvalue = std::max(max_corrected_pvalue, pvalue * correcting_multiplier);
    pvalue = std::min(pvalue, 1.0);
    max_corrected_pvalue = pvalue;
    --correcting_multiplier;
  };
  return {std::begin(corrected_pvalues), std::end(corrected_pvalues)};
}

}  // namespace orbit_statistics

#endif  // STATISTICS_MULTIPLICITY_CORRECTION_H_
