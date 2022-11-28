// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <vector>

#include "Statistics/MultiplicityCorrection.h"
#include "TestUtils/ContainerHelpers.h"

namespace orbit_statistics {

using testing::DoubleNear;
using testing::SizeIs;

using orbit_test_utils::MakeMap;

constexpr size_t kTestsNum = 4;
constexpr std::array<double, kTestsNum> kPvalues = {0.1, 0.2, 0.3, 0.02};

static void ExpectCorrectedPvaluesEq(const absl::flat_hash_map<int, double>& actual,
                                     const absl::flat_hash_map<int, double>& expected) {
  EXPECT_THAT(actual, SizeIs(expected.size()));
  for (const auto& [key, expected_corrected_pvalue] : expected) {
    constexpr double kTolerance = 1e-3;
    EXPECT_THAT(actual.at(key), DoubleNear(expected_corrected_pvalue, kTolerance));
  }
}

using Key = int;

template <typename Container>
std::vector<Key> MakeKeys(const Container& container) {
  std::vector<Key> keys;
  for (size_t i = 1; i <= container.size(); ++i) {
    keys.push_back(i);
  }
  return keys;
}

template <auto Correction, typename Container, typename AnotherContainer>
static void ExpectCorrectionIsCorrect(const Container& pvalues,
                                      const AnotherContainer& expected_corrected_pvalues) {
  const std::vector<Key> keys = MakeKeys(pvalues);
  const absl::flat_hash_map<Key, double> key_to_pvalue = MakeMap(keys, pvalues);
  const absl::flat_hash_map<Key, double> expected_key_to_corrected_pvalues =
      MakeMap(keys, expected_corrected_pvalues);
  const absl::flat_hash_map<Key, double> actual_key_to_corrected_pvalues =
      Correction(key_to_pvalue);
  ExpectCorrectedPvaluesEq(actual_key_to_corrected_pvalues, expected_key_to_corrected_pvalues);
}

TEST(MultiplicityCorrection, BonferroniCorrectionIsCorrect) {
  std::array<double, kTestsNum> expected_pvalues{};
  std::transform(std::begin(kPvalues), std::end(kPvalues), std::begin(expected_pvalues),
                 [](const double pvalue) { return pvalue * kTestsNum; });

  ExpectCorrectionIsCorrect<BonferroniCorrection<Key>>(kPvalues, expected_pvalues);
}

TEST(MultiplicityCorrection, HolmBonferroniCorrectionIsCorrect) {
  constexpr std::array<double, kTestsNum> kExpectedCorrectedPvalue = {0.30, 0.40, 0.40, 0.08};
  ExpectCorrectionIsCorrect<HolmBonferroniCorrection<Key>>(kPvalues, kExpectedCorrectedPvalue);

  ExpectCorrectionIsCorrect<HolmBonferroniCorrection<Key>>(std::vector<double>{},
                                                           std::vector<double>{});

  ExpectCorrectionIsCorrect<HolmBonferroniCorrection<Key>>(
      std::vector<double>{1e-3, 5e-4, 0.02, 1e-5, 1, 1, 0.3},
      std::vector<double>{5e-03, 3e-03, 8e-02, 7e-05, 1e+00, 1e+00, 9e-01});

  ExpectCorrectionIsCorrect<HolmBonferroniCorrection<Key>>(
      std::vector<double>{0, 0, 0, 0.05, 1, 1, 1}, std::vector<double>{0, 0, 0, 0.2, 1, 1, 1});
}

}  // namespace orbit_statistics