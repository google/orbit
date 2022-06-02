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
static const std::vector<std::string> kBaselineFunctionNames = {"foo()", "bar()", "biz()"};
static const std::vector<std::string> kComparisonFunctionNames = {"foo()", "bar()", "fiz()"};

template <typename Container>
auto Commons(const Container& a, const Container& b) {
  typedef typename Container::value_type E;
  absl::flat_hash_set<E> a_set(std::begin(a), std::end(a));
  std::vector<E> result;
  std::copy_if(std::begin(b), std::end(b), std::back_inserter(result),
               [&a_set](const E& element) { return a_set.contains(element); });
  return result;
}

static const std::vector<std::string> kCommonFunctionNames =
    Commons(kBaselineFunctionNames, kComparisonFunctionNames);

template <typename Container, typename AnotherContainer>
auto MakeMap(const Container& keys, const AnotherContainer& values) {
  typedef typename Container::value_type K;
  typedef typename AnotherContainer::value_type V;

  absl::flat_hash_map<K, V> result;
  std::transform(std::begin(keys), std::end(keys), std::begin(values),
                 std::inserter(result, std::begin(result)),
                 [](const K& k, const V& v) { return std::make_pair(k, v); });
  return result;
}

static const absl::flat_hash_map<uint64_t, std::string> kBaselineAddressToName =
    MakeMap(kBaselineFunctionAddresses, kBaselineFunctionNames);
static const absl::flat_hash_map<uint64_t, std::string> kComparisonAddressToName =
    MakeMap(kComparisonFunctionAddresses, kComparisonFunctionNames);

void ExpectCorrectNames(const absl::flat_hash_map<uint64_t, uint64_t>& address_to_id,
                        const absl::flat_hash_map<uint64_t, std::string>& id_to_name,
                        const absl::flat_hash_map<uint64_t, std::string>& address_to_name) {
  for (const auto& [address, id] : address_to_id) {
    if (id_to_name.contains(address)) {
      EXPECT_EQ(id_to_name.at(address), address_to_name.at(address));
    }
  }
}

template <typename K, typename V>
std::vector<V> Values(const absl::flat_hash_map<K, V>& map) {
  std::vector<V> values;
  std::transform(std::begin(map), std::end(map), std::back_inserter(values),
                 [](const std::pair<K, V> pair) { return pair.second; });
  return values;
}

TEST(BaselineAndComparisonTest, BaselineAndComparisonHelperIsCorrect) {
  const auto [baseline_address_to_sampled_function_id, comparison_address_to_sampled_function_id,
              sampled_function_id_id_to_name] =
      AssignSampledFunctionIds(kBaselineAddressToName, kComparisonAddressToName);

  EXPECT_EQ(baseline_address_to_sampled_function_id.size(), kCommonFunctionNames.size());
  EXPECT_EQ(comparison_address_to_sampled_function_id.size(), kCommonFunctionNames.size());
  EXPECT_EQ(sampled_function_id_id_to_name.size(), kCommonFunctionNames.size());

  ExpectCorrectNames(baseline_address_to_sampled_function_id, sampled_function_id_id_to_name,
                     kBaselineAddressToName);
  ExpectCorrectNames(comparison_address_to_sampled_function_id, sampled_function_id_id_to_name,
                     kComparisonAddressToName);

  EXPECT_THAT(
      Values(baseline_address_to_sampled_function_id),
      testing::UnorderedElementsAreArray(Values(comparison_address_to_sampled_function_id)));
}

}  // namespace orbit_mizar_data