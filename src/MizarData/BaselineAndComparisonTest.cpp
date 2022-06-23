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
#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/BaselineAndComparison.h"
#include "OrbitBase/ThreadConstants.h"

using ::testing::DoubleNear;
using ::testing::UnorderedElementsAreArray;

template <typename T>
using Baseline = ::orbit_mizar_base::Baseline<T>;

template <typename T>
using Comparison = ::orbit_mizar_base::Comparison<T>;

using ::orbit_mizar_base::MakeBaseline;
using ::orbit_mizar_base::MakeComparison;

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

constexpr size_t kSfidCount = 3;
constexpr SFID kSfidFirst = SFID(1);
constexpr SFID kSfidSecond = SFID(2);
constexpr SFID kSfidThird = SFID(3);
constexpr std::array<SFID, kSfidCount> kSfids = {kSfidFirst, kSfidSecond, kSfidThird};
const absl::flat_hash_map<SFID, std::string> kSfidToName = MakeMap(kSfids, kBaselineFunctionNames);

const std::vector<std::vector<SFID>> kCallstacks = {
    std::vector<SFID>{kSfidThird, kSfidSecond, kSfidFirst}, {kSfidSecond}, {}};

const std::vector<uint64_t> kFrameTrackActiveTimes = {300, 100, 200};

constexpr double kStatistic = 1.234;
constexpr std::array<double, kSfidCount> kPvalues = {0.01, 0.02, 0.05};
const std::array<double, kSfidCount> kCorrectedPvalues = [] {
  std::array<double, kSfidCount> result{};
  std::transform(std::begin(kPvalues), std::end(kPvalues), std::begin(result),
                 [](const double pvalue) { return pvalue * 2; });
  return result;
}();
const absl::flat_hash_map<SFID, double> kSfidToPvalue = MakeMap(kSfids, kPvalues);
const absl::flat_hash_map<SFID, double> kSfidToCorrectedPvalue = MakeMap(kSfids, kCorrectedPvalues);

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
      const absl::flat_hash_set<uint32_t>& /*tids*/, uint64_t /*frame_track_scope_id*/,
      uint64_t /*min_relative_timestamp_ns*/, uint64_t /*max_relative_timestamp_ns*/) const {
    return frame_track_active_times_;
  };

 private:
  std::vector<std::vector<SFID>> callstacks_;
  std::vector<uint64_t> frame_track_active_times_;
};

class MockFunctionTimeComparator {
 public:
  MockFunctionTimeComparator(
      const Baseline<SamplingCounts>& /*baseline_counts*/,
      const Baseline<orbit_client_data::ScopeStats>& /*baseline_frame_stats*/,
      const Comparison<SamplingCounts>& /*comparison_counts*/,
      const Comparison<orbit_client_data::ScopeStats>& /*comparison_frame_stats*/) {}

  [[nodiscard]] ComparisonResult Compare(SFID sfid) const {
    return {kStatistic, kSfidToPvalue.at(sfid)};
  };
};

}  // namespace

// It's not even a correction in statistical sense. Good only for mocking.
[[nodiscard]] static absl::flat_hash_map<SFID, double> MockCorrection(
    const absl::flat_hash_map<SFID, double>& pvalues) {
  absl::flat_hash_map<SFID, double> result;
  std::transform(std::begin(pvalues), std::end(pvalues), std::inserter(result, std::begin(result)),
                 [](const auto& key_to_pvalue) {
                   const SFID sfid = key_to_pvalue.first;
                   return std::make_pair(sfid, kSfidToCorrectedPvalue.at(sfid));
                 });
  return result;
}

TEST(BaselineAndComparisonTest, MakeSamplingWithFrameTrackReportIsCorrect) {
  auto full = MakeBaseline<MockPairedData>(kCallstacks, kFrameTrackActiveTimes);
  auto empty =
      MakeComparison<MockPairedData>(std::vector<std::vector<SFID>>{}, std::vector<uint64_t>{});

  BaselineAndComparisonTmpl<MockPairedData, MockFunctionTimeComparator, MockCorrection> bac(
      std::move(full), std::move(empty), {kSfidToName});
  const SamplingWithFrameTrackComparisonReport report = bac.MakeSamplingWithFrameTrackReport(
      MakeBaseline<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
          absl::flat_hash_set<uint32_t>{orbit_base::kAllProcessThreadsTid}, 0, 1, 1),
      MakeComparison<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
          absl::flat_hash_set<uint32_t>{orbit_base::kAllProcessThreadsTid}, 0, 1, 1));

  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetTotalCallstacks(), kCallstacks.size());

  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetExclusiveCount(kSfidFirst), 0);
  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetExclusiveCount(kSfidSecond), 1);
  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetExclusiveCount(kSfidThird), 1);

  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetInclusiveCount(kSfidFirst), 1);
  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetInclusiveCount(kSfidSecond), 2);
  EXPECT_EQ(report.GetSamplingCounts<Baseline>()->GetInclusiveCount(kSfidThird), 1);

  EXPECT_EQ(report.GetSamplingCounts<Comparison>()->GetTotalCallstacks(), 0);
  for (const SFID sfid : kSfids) {
    EXPECT_EQ(report.GetSamplingCounts<Comparison>()->GetExclusiveCount(sfid), 0);
    EXPECT_EQ(report.GetSamplingCounts<Comparison>()->GetInclusiveCount(sfid), 0);

    CorrectedComparisonResult comparision_result = report.GetComparisonResult(sfid);
    constexpr double kTolerance = 1e-6;
    EXPECT_THAT(comparision_result.statistic, DoubleNear(kStatistic, kTolerance));

    EXPECT_THAT(comparision_result.corrected_pvalue,
                DoubleNear(kSfidToCorrectedPvalue.at(sfid), kTolerance));
  }

  constexpr uint64_t kExpectedFullActiveFrameTime = 200;
  EXPECT_EQ(report.GetFrameTrackStats<Baseline>()->ComputeAverageTimeNs(),
            kExpectedFullActiveFrameTime);
  EXPECT_EQ(report.GetFrameTrackStats<Comparison>()->ComputeAverageTimeNs(), 0);

  constexpr double kExpectedFullActiveFrameTimeVariance = 6666.66666;
  EXPECT_THAT(report.GetFrameTrackStats<Baseline>()->variance_ns(),
              DoubleNear(kExpectedFullActiveFrameTimeVariance, 1e-3));
  EXPECT_THAT(report.GetFrameTrackStats<Comparison>()->variance_ns(), DoubleNear(0, 1e-3));

  EXPECT_EQ(report.GetFrameTrackStats<Baseline>()->count(), kFrameTrackActiveTimes.size());
  EXPECT_EQ(report.GetFrameTrackStats<Comparison>()->count(), 0);
}

}  // namespace orbit_mizar_data