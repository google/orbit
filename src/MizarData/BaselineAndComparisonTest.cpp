// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "BaselineAndComparisonHelper.h"
#include "MizarData/BaselineAndComparison.h"
#include "OrbitBase/ThreadConstants.h"

using ::testing::DoubleNear;
using ::testing::UnorderedElementsAreArray;

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
              UnorderedElementsAreArray(Values(comparison_address_to_sfid)));
}

constexpr size_t kSFIDCnt = 3;
constexpr SFID kSFIDFirst = SFID(1);
constexpr SFID kSFIDSecond = SFID(2);
constexpr SFID kSFIDThird = SFID(3);
constexpr std::array<SFID, kSFIDCnt> kSFIDs = {kSFIDFirst, kSFIDSecond, kSFIDThird};

const std::vector<std::vector<SFID>> kCallstacks = {
    std::vector<SFID>{kSFIDFirst, kSFIDSecond, kSFIDThird}, {kSFIDSecond}, {}};

const std::vector<uint64_t> kFrameTrackActiveTimes = {300, 100, 200};

namespace {
class MockPairedData {
 public:
  explicit MockPairedData(std::vector<std::vector<SFID>> callstacks,
                          std::vector<uint64_t> frame_track_active_times)
      : callstacks_(std::move(callstacks)),
        frame_track_active_times_(std::move(frame_track_active_times)) {}

  template <typename Action>
  void ForEachCallstackEvent(uint32_t /*tid*/, uint64_t /*min_timestamp*/,
                             uint64_t /*max_timestamp*/, Action&& action) const {
    std::for_each(std::begin(callstacks_), std::end(callstacks_), std::forward<Action>(action));
  }

  [[nodiscard]] std::vector<uint64_t> ActiveInvocationTimes(
      const absl::flat_hash_set<uint32_t>& /*_*/, uint64_t /*_*/, uint64_t /*_*/,
      uint64_t /*_*/) const {
    return frame_track_active_times_;
  };

 private:
  std::vector<std::vector<SFID>> callstacks_;
  std::vector<uint64_t> frame_track_active_times_;
};
}  // namespace

TEST(BaselineAndComparisonTest, MakeSamplingWithFrameTrackReportIsCorrect) {
  auto full = MakeBaseline<MockPairedData>(kCallstacks, kFrameTrackActiveTimes);
  auto empty =
      MakeComparison<MockPairedData>(std::vector<std::vector<SFID>>{}, std::vector<uint64_t>{});

  BaselineAndComparisonTmpl<MockPairedData> bac(std::move(full), std::move(empty), {});
  const SamplingWithFrameTrackComparisonReport report = bac.MakeSamplingWithFrameTrackReport(
      orbit_mizar_data::MakeBaseline<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
          absl::flat_hash_set<uint32_t>{orbit_base::kAllProcessThreadsTid}, 0, 1, 1),
      orbit_mizar_data::MakeComparison<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
          absl::flat_hash_set<uint32_t>{orbit_base::kAllProcessThreadsTid}, 0, 1, 1));

  EXPECT_EQ(report.baseline_sampling_counts.GetTotalCallstacks(), kCallstacks.size());

  EXPECT_EQ(report.baseline_sampling_counts.GetExclusiveCount(kSFIDFirst), 0);
  EXPECT_EQ(report.baseline_sampling_counts.GetExclusiveCount(kSFIDSecond), 1);
  EXPECT_EQ(report.baseline_sampling_counts.GetExclusiveCount(kSFIDThird), 1);

  EXPECT_EQ(report.baseline_sampling_counts.GetInclusiveCount(kSFIDFirst), 1);
  EXPECT_EQ(report.baseline_sampling_counts.GetInclusiveCount(kSFIDSecond), 2);
  EXPECT_EQ(report.baseline_sampling_counts.GetInclusiveCount(kSFIDThird), 1);

  EXPECT_EQ(report.comparison_sampling_counts.GetTotalCallstacks(), 0);
  for (const SFID sfid : kSFIDs) {
    EXPECT_EQ(report.comparison_sampling_counts.GetExclusiveCount(sfid), 0);
    EXPECT_EQ(report.comparison_sampling_counts.GetInclusiveCount(sfid), 0);
  }

  constexpr uint64_t kExpectedFullActiveFrameTime = 200;
  EXPECT_EQ(report.baseline_frame_track_stats.ComputeAverageTimeNs(), kExpectedFullActiveFrameTime);
  EXPECT_EQ(report.comparison_frame_track_stats.ComputeAverageTimeNs(), 0);

  constexpr double kExpectedFullActiveFrameTimeVariance = 6666.66666;
  EXPECT_THAT(report.baseline_frame_track_stats.variance_ns(),
              DoubleNear(kExpectedFullActiveFrameTimeVariance, 1e-3));
  EXPECT_THAT(report.comparison_frame_track_stats.variance_ns(), DoubleNear(0, 1e-3));

  EXPECT_EQ(report.baseline_frame_track_stats.count(), kFrameTrackActiveTimes.size());
  EXPECT_EQ(report.comparison_frame_track_stats.count(), 0);
}

}  // namespace orbit_mizar_data