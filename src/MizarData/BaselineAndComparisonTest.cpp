// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "BaselineAndComparisonHelper.h"
#include "MizarData/BaselineAndComparison.h"

namespace orbit_mizar_data {

constexpr size_t kFunctionNum = 3;
constexpr std::array<uint64_t, kFunctionNum> kBaselineFunctionAddresses = {0xF00D, 0xBEAF, 0xDEAF};
constexpr std::array<uint64_t, kFunctionNum> kComparisonFunctionAddresses = {0x0FF, 0xCAFE, 0xDEA};
const std::array<std::string, kFunctionNum> kBaselineFunctionNames = {"foo()", "bar()", "biz()"};
const std::array<std::string, kFunctionNum> kComparisonFunctionNames = {"foo()", "bar()", "fiz()"};

template <typename Container>
[[nodiscard]] static auto Commons(const Container& a, const Container& b) {
  using E = typename Container::value_type;
  using std::begin;
  using std::end;

  absl::flat_hash_set<E> a_set(begin(a), end(a));
  std::vector<E> result;
  std::copy_if(begin(b), end(b), std::back_inserter(result),
               [&a_set](const E& element) { return a_set.contains(element); });
  return result;
}

const std::vector<std::string> kCommonFunctionNames =
    Commons(kBaselineFunctionNames, kComparisonFunctionNames);

template <typename Container, typename AnotherContainer>
auto MakeMap(const Container& keys, const AnotherContainer& values) {
  using K = typename Container::value_type;
  using V = typename AnotherContainer::value_type;
  using std::begin;
  using std::end;

  absl::flat_hash_map<K, V> result;
  std::transform(begin(keys), end(keys), begin(values), std::inserter(result, std::begin(result)),
                 [](const K& k, const V& v) { return std::make_pair(k, v); });
  return result;
}

const absl::flat_hash_map<uint64_t, std::string> kBaselineAddressToName =
    MakeMap(kBaselineFunctionAddresses, kBaselineFunctionNames);
const absl::flat_hash_map<uint64_t, std::string> kComparisonAddressToName =
    MakeMap(kComparisonFunctionAddresses, kComparisonFunctionNames);

static void ExpectCorrectNames(const absl::flat_hash_map<uint64_t, SFID>& address_to_sfid,
                               const absl::flat_hash_map<SFID, std::string>& sfid_to_name,
                               const absl::flat_hash_map<uint64_t, std::string>& address_to_name) {
  for (const auto& [address, sfid] : address_to_sfid) {
    EXPECT_TRUE(sfid_to_name.contains(sfid));
    EXPECT_EQ(sfid_to_name.at(sfid), address_to_name.at(address));
  }
}

template <typename K, typename V>
[[nodiscard]] static std::vector<V> Values(const absl::flat_hash_map<K, V>& map) {
  std::vector<V> values;
  std::transform(std::begin(map), std::end(map), std::back_inserter(values),
                 [](const std::pair<K, V> pair) { return pair.second; });
  return values;
}

TEST(BaselineAndComparisonTest, BaselineAndComparisonHelperIsCorrect) {
  const auto [baseline_address_to_sfid, comparison_address_to_sfid, sfid_to_name] =
      AssignSampledFunctionIds(kBaselineAddressToName, kComparisonAddressToName);

  EXPECT_EQ(baseline_address_to_sfid.size(), kCommonFunctionNames.size());
  EXPECT_EQ(comparison_address_to_sfid.size(), kCommonFunctionNames.size());
  EXPECT_EQ(sfid_to_name.size(), kCommonFunctionNames.size());

  ExpectCorrectNames(baseline_address_to_sfid, sfid_to_name, kBaselineAddressToName);
  ExpectCorrectNames(comparison_address_to_sfid, sfid_to_name, kComparisonAddressToName);

  EXPECT_THAT(Values(baseline_address_to_sfid),
              testing::UnorderedElementsAreArray(Values(comparison_address_to_sfid)));
}

}  // namespace orbit_mizar_data