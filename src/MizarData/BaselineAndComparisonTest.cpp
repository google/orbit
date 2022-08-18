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
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStats.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarBase/Time.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarStatistics/ActiveFunctionTimePerFrameComparator.h"
#include "OrbitBase/ThreadConstants.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_mizar_base::AbsoluteAddress;
using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::MakeBaseline;
using ::orbit_mizar_base::MakeComparison;
using ::orbit_mizar_base::RelativeTimeNs;
using ::orbit_mizar_base::SFID;
using ::orbit_mizar_base::TID;
using ::orbit_test_utils::Commons;
using ::orbit_test_utils::MakeMap;
using ::testing::DoubleNear;
using ::testing::UnorderedElementsAreArray;

namespace orbit_mizar_data {

constexpr size_t kFunctionNum = 3;
constexpr std::array<uint64_t, kFunctionNum> kBaselineFunctionAddresses = {0xF00D, 0xBEAF, 0xDEAF};
constexpr std::array<uint64_t, kFunctionNum> kComparisonFunctionAddresses = {0x0FF, 0xCAFE, 0xDEA};
const std::array<std::string, kFunctionNum> kBaselineFunctionNames = {"foo()", "bar()", "biz()"};
const std::array<std::string, kFunctionNum> kComparisonFunctionNames = {"foo()", "bar()", "fiz()"};

const std::vector<std::string> kCommonFunctionNames =
    Commons(kBaselineFunctionNames, kComparisonFunctionNames);

template <size_t N>
static absl::flat_hash_map<AbsoluteAddress, std::string> MakeAddressToNameMap(
    const std::array<uint64_t, N>& raw_addresses, const std::array<std::string, N>& names) {
  std::array<AbsoluteAddress, N> addresses;
  absl::c_transform(raw_addresses, std::begin(addresses),
                    [](uint64_t raw) { return AbsoluteAddress(raw); });
  return MakeMap(addresses, names);
}

const absl::flat_hash_map<AbsoluteAddress, std::string> kBaselineAddressToName =
    MakeAddressToNameMap(kBaselineFunctionAddresses, kBaselineFunctionNames);
const absl::flat_hash_map<AbsoluteAddress, std::string> kComparisonAddressToName =
    MakeAddressToNameMap(kComparisonFunctionAddresses, kComparisonFunctionNames);

static void ExpectCorrectNames(
    const absl::flat_hash_map<AbsoluteAddress, SFID>& address_to_sfid,
    const absl::flat_hash_map<SFID, std::string>& sfid_to_name,
    const absl::flat_hash_map<AbsoluteAddress, std::string>& address_to_name) {
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

const orbit_client_data::ScopeStats kNonEmptyScopeStats = [] {
  orbit_client_data::ScopeStats result;
  for (uint64_t time : {300, 100, 200}) {
    result.UpdateStats(time);
  }
  return result;
}();

constexpr orbit_client_data::ScopeStats kEmptyScopeStats;

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
                          orbit_client_data::ScopeStats frame_track_stats)
      : callstacks_(std::move(callstacks)), frame_track_stats_(frame_track_stats) {}

  template <typename Action>
  void ForEachCallstackEvent(TID /*tid*/, RelativeTimeNs /*min_timestamp*/,
                             RelativeTimeNs /*max_timestamp*/, Action&& action) const {
    std::for_each(std::begin(callstacks_), std::end(callstacks_), std::forward<Action>(action));
  }

  [[nodiscard]] orbit_client_data::ScopeStats ActiveInvocationTimeStats(
      const absl::flat_hash_set<TID>& /*tids*/, FrameTrackId /*frame_track_scope_id*/,
      RelativeTimeNs /*min_relative_timestamp_ns*/,
      RelativeTimeNs /*max_relative_timestamp_ns*/) const {
    return frame_track_stats_;
  };

 private:
  std::vector<std::vector<SFID>> callstacks_;
  orbit_client_data::ScopeStats frame_track_stats_;
};

class MockFunctionTimeComparator {
 public:
  MockFunctionTimeComparator(
      const Baseline<SamplingCounts>& /*baseline_counts*/,
      const Baseline<orbit_client_data::ScopeStats>& /*baseline_frame_stats*/,
      const Comparison<SamplingCounts>& /*comparison_counts*/,
      const Comparison<orbit_client_data::ScopeStats>& /*comparison_frame_stats*/) {}

  [[nodiscard]] orbit_mizar_statistics::ComparisonResult Compare(SFID sfid) const {
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

static void ExpectScopeStatsEq(const orbit_client_data::ScopeStats a,
                               const orbit_client_data::ScopeStats b) {
  EXPECT_EQ(a.ComputeAverageTimeNs(), b.ComputeAverageTimeNs()); 
  EXPECT_EQ(a.variance_ns(), b.variance_ns());
  EXPECT_EQ(a.count(), b.count());
}

TEST(BaselineAndComparisonTest, MakeSamplingWithFrameTrackReportIsCorrect) {
  auto full = MakeBaseline<MockPairedData>(kCallstacks, kNonEmptyScopeStats);
  auto empty = MakeComparison<MockPairedData>(std::vector<std::vector<SFID>>{}, kEmptyScopeStats);

  BaselineAndComparisonTmpl<MockPairedData, MockFunctionTimeComparator, MockCorrection> bac(
      std::move(full), std::move(empty), {kSfidToName});
  const SamplingWithFrameTrackComparisonReport report = bac.MakeSamplingWithFrameTrackReport(
      MakeBaseline<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
          absl::flat_hash_set<TID>{TID(orbit_base::kAllProcessThreadsTid)}, RelativeTimeNs(0),
          FrameTrackId(ScopeId(1))),
      MakeComparison<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
          absl::flat_hash_set<TID>{TID(orbit_base::kAllProcessThreadsTid)}, RelativeTimeNs(0),
          FrameTrackId(ScopeId(1))));

  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetTotalCallstacks(), kCallstacks.size());

  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetExclusiveCount(kSfidFirst), 0);
  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetExclusiveCount(kSfidSecond), 1);
  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetExclusiveCount(kSfidThird), 1);

  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetInclusiveCount(kSfidFirst), 1);
  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetInclusiveCount(kSfidSecond), 2);
  EXPECT_EQ(report.GetBaselineSamplingCounts()->GetInclusiveCount(kSfidThird), 1);

  EXPECT_EQ(report.GetComparisonSamplingCounts()->GetTotalCallstacks(), 0);
  for (const SFID sfid : kSfids) {
    EXPECT_EQ(report.GetComparisonSamplingCounts()->GetExclusiveCount(sfid), 0);
    EXPECT_EQ(report.GetComparisonSamplingCounts()->GetInclusiveCount(sfid), 0);

    CorrectedComparisonResult comparision_result = report.GetComparisonResult(sfid);
    constexpr double kTolerance = 1e-6;
    EXPECT_THAT(comparision_result.statistic, DoubleNear(kStatistic, kTolerance));

    EXPECT_THAT(comparision_result.corrected_pvalue,
                DoubleNear(kSfidToCorrectedPvalue.at(sfid), kTolerance));
  }

  ExpectScopeStatsEq(*report.GetBaselineFrameTrackStats(), kNonEmptyScopeStats);
  ExpectScopeStatsEq(*report.GetComparisonFrameTrackStats(), kEmptyScopeStats);
}

}  // namespace orbit_mizar_data