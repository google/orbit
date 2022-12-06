// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "BaselineAndComparisonHelper.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStats.h"
#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarStatistics/ActiveFunctionTimePerFrameComparator.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/Typedef.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_mizar_base::AbsoluteAddress;
using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::BaselineAndComparisonFunctionSymbols;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::FunctionSymbol;
using ::orbit_mizar_base::MakeBaseline;
using ::orbit_mizar_base::MakeComparison;
using ::orbit_mizar_base::RelativeTimeNs;
using ::orbit_mizar_base::SampledFunctionId;
using ::orbit_mizar_base::TID;
using ::orbit_test_utils::Commons;
using ::orbit_test_utils::MakeMap;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoubleNear;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::UnorderedElementsAreArray;

namespace orbit_mizar_data {

constexpr size_t kFunctionNum = 3;
constexpr std::array<uint64_t, kFunctionNum> kBaselineFunctionAddresses = {0xF00D, 0xBEAF, 0xDEAF};
constexpr std::array<uint64_t, kFunctionNum> kComparisonFunctionAddresses = {0x0FF, 0xCAFE, 0xDEA};
const std::array<std::string, kFunctionNum> kBaselineFunctionNames = {"foo()", "bar()", "biz()"};
const std::array<std::string, kFunctionNum> kComparisonFunctionNames = {"foo()", "bar()", "fiz()"};

const std::array<std::string, kFunctionNum> kModuleNames = {"fooM", "barM", "bizM"};

const std::vector<std::string> kCommonFunctionNames =
    Commons(kBaselineFunctionNames, kComparisonFunctionNames);

template <size_t N>
static std::array<FunctionSymbol, N> MakeFunctionSymbols(
    const std::array<std::string, N>& functions) {
  std::array<FunctionSymbol, N> symbols;
  absl::c_transform(functions, kModuleNames, std::begin(symbols),
                    [](std::string_view function, std::string_view module) {
                      return FunctionSymbol{std::string{function}, std::string{module}};
                    });
  return symbols;
}

template <size_t N>
static absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> MakeAddressToSymbolMap(
    const std::array<uint64_t, N>& raw_addresses,
    const std::array<std::string, N>& function_names) {
  std::array<AbsoluteAddress, N> addresses;
  absl::c_transform(raw_addresses, std::begin(addresses),
                    [](uint64_t raw) { return AbsoluteAddress(raw); });
  std::array<FunctionSymbol, N> symbols = MakeFunctionSymbols(function_names);
  return MakeMap(addresses, symbols);
}

const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> kBaselineAddressToSymbol =
    MakeAddressToSymbolMap(kBaselineFunctionAddresses, kBaselineFunctionNames);
const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> kComparisonAddressToSymbol =
    MakeAddressToSymbolMap(kComparisonFunctionAddresses, kComparisonFunctionNames);

static void ExpectCorrectNames(
    const absl::flat_hash_map<AbsoluteAddress, SampledFunctionId>& address_to_sfid,
    const absl::flat_hash_map<SampledFunctionId, BaselineAndComparisonFunctionSymbols>&
        sfid_to_symbols,
    const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& address_to_symbol) {
  for (const auto& [address, sfid] : address_to_sfid) {
    EXPECT_TRUE(sfid_to_symbols.contains(sfid));
    const BaselineAndComparisonFunctionSymbols& symbols = sfid_to_symbols.at(sfid);
    EXPECT_EQ(symbols.baseline_function_symbol->function_name,
              address_to_symbol.at(address).function_name);
    EXPECT_EQ(symbols.comparison_function_symbol->function_name,
              address_to_symbol.at(address).function_name);
  }
}

template <typename K, typename V>
[[nodiscard]] static std::vector<V> Values(const absl::flat_hash_map<K, V>& map) {
  std::vector<V> values;
  std::transform(std::begin(map), std::end(map), std::back_inserter(values),
                 [](const std::pair<K, V> pair) { return pair.second; });
  return values;
}

MATCHER_P(FunctionSymbolEq, expected, "") {
  const FunctionSymbol& symbol = std::get<0>(arg);
  return symbol.function_name == expected.function_name &&
         symbol.module_file_name == expected.module_file_name;
}

namespace {
class MockFunctionSymbolToKey {
 public:
  MockFunctionSymbolToKey() {
    for (const auto& [unused_address, symbol] : kBaselineAddressToSymbol) {
      ExpectCall(symbol);
    }

    // The only symbol in comparison data, that isn't also present in baseline
    ExpectCall(FunctionSymbol{"fiz()", "bizM"});
  }

  MOCK_METHOD(int, GetKey, (FunctionSymbol), (const));

 private:
  void ExpectCall(const FunctionSymbol& symbol) {
    EXPECT_CALL(*this, GetKey(_))
        .With(FunctionSymbolEq(symbol))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(next_key_++));
  }

  int next_key_ = 1;
};

}  // namespace

TEST(BaselineAndComparisonTest, BaselineAndComparisonHelperIsCorrect) {
  BaselineAndComparisonHelperTmpl<MockFunctionSymbolToKey, int> helper;

  const auto [baseline_address_to_sfid, comparison_address_to_sfid, sfid_to_symbols] =
      helper.AssignSampledFunctionIds(kBaselineAddressToSymbol, kComparisonAddressToSymbol);

  EXPECT_EQ(baseline_address_to_sfid.size(), kCommonFunctionNames.size());
  EXPECT_EQ(comparison_address_to_sfid.size(), kCommonFunctionNames.size());
  EXPECT_EQ(sfid_to_symbols.size(), kCommonFunctionNames.size());

  ExpectCorrectNames(baseline_address_to_sfid, sfid_to_symbols, kBaselineAddressToSymbol);
  ExpectCorrectNames(comparison_address_to_sfid, sfid_to_symbols, kComparisonAddressToSymbol);

  EXPECT_THAT(Values(baseline_address_to_sfid),
              UnorderedElementsAreArray(Values(comparison_address_to_sfid)));
}

constexpr size_t kSfidCount = 3;
constexpr SampledFunctionId kSfidFirst = SampledFunctionId(1);
constexpr SampledFunctionId kSfidSecond = SampledFunctionId(2);
constexpr SampledFunctionId kSfidThird = SampledFunctionId(3);
constexpr std::array<SampledFunctionId, kSfidCount> kSfids = {kSfidFirst, kSfidSecond, kSfidThird};

const absl::flat_hash_map<SampledFunctionId, BaselineAndComparisonFunctionSymbols>
    kFunctionSymbols = [] {
      absl::flat_hash_map<SampledFunctionId, BaselineAndComparisonFunctionSymbols> result;
      absl::c_transform(kSfids, MakeFunctionSymbols(kBaselineFunctionNames),
                        std::inserter(result, std::begin(result)),
                        [](SampledFunctionId sfid, const FunctionSymbol& symbol) {
                          BaselineAndComparisonFunctionSymbols symbols{
                              Baseline<FunctionSymbol>(symbol), Comparison<FunctionSymbol>(symbol)};
                          return std::make_pair(sfid, symbols);
                        });
      return result;
    }();

const std::vector<std::vector<SampledFunctionId>> kCallstacks = {
    std::vector<SampledFunctionId>{kSfidThird, kSfidSecond, kSfidFirst}, {kSfidSecond}, {}};

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
const absl::flat_hash_map<SampledFunctionId, double> kSfidToPvalue = MakeMap(kSfids, kPvalues);
const absl::flat_hash_map<SampledFunctionId, double> kSfidToCorrectedPvalue =
    MakeMap(kSfids, kCorrectedPvalues);

namespace {
class MockPairedData {
 public:
  explicit MockPairedData(std::vector<std::vector<SampledFunctionId>> callstacks,
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
  std::vector<std::vector<SampledFunctionId>> callstacks_;
  orbit_client_data::ScopeStats frame_track_stats_;
};

class MockFunctionTimeComparator {
 public:
  MockFunctionTimeComparator(
      const Baseline<SamplingCounts>& /*baseline_counts*/,
      const Baseline<orbit_client_data::ScopeStats>& /*baseline_frame_stats*/,
      const Comparison<SamplingCounts>& /*comparison_counts*/,
      const Comparison<orbit_client_data::ScopeStats>& /*comparison_frame_stats*/) {}

  [[nodiscard]] static orbit_mizar_statistics::ComparisonResult Compare(SampledFunctionId sfid) {
    return {kStatistic, kSfidToPvalue.at(sfid)};
  };
};

}  // namespace

// It's not even a correction in statistical sense. Good only for mocking.
[[nodiscard]] static absl::flat_hash_map<SampledFunctionId, double> MockCorrection(
    const absl::flat_hash_map<SampledFunctionId, double>& pvalues) {
  absl::flat_hash_map<SampledFunctionId, double> result;
  std::transform(std::begin(pvalues), std::end(pvalues), std::inserter(result, std::begin(result)),
                 [](const auto& key_to_pvalue) {
                   const SampledFunctionId sfid = key_to_pvalue.first;
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
  auto empty = MakeComparison<MockPairedData>(std::vector<std::vector<SampledFunctionId>>{},
                                              kEmptyScopeStats);

  BaselineAndComparisonTmpl<MockPairedData, MockFunctionTimeComparator, MockCorrection> bac(
      std::move(full), std::move(empty), kFunctionSymbols);
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
  for (const SampledFunctionId sfid : kSfids) {
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