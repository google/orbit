// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <QList>
#include <QMetaType>
#include <QModelIndex>
#include <QSignalSpy>
#include <QString>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarModels/SamplingWithFrameTrackReportModel.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::FunctionSymbol;
using ::testing::Ne;
using ::testing::StrNe;
using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;
using SFID = ::orbit_mizar_base::SampledFunctionId;
using Counts = ::orbit_mizar_data::SamplingCounts;
using ::orbit_mizar_base::BaselineAndComparisonFunctionSymbols;
using ::orbit_mizar_base::MakeBaseline;
using ::orbit_mizar_base::MakeComparison;
using ::orbit_test_utils::MakeMap;
using ::testing::Contains;
using ::testing::DoubleNear;

namespace {
class MockCounts {
 public:
  MockCounts(uint64_t callstack_count, absl::flat_hash_map<SFID, uint64_t> counts)
      : callstack_count_(callstack_count), counts_(std::move(counts)) {}

  [[nodiscard]] uint64_t GetExclusiveCount(SFID sfid) const { return counts_.at(sfid); }
  [[nodiscard]] double GetExclusiveRate(SFID sfid) const {
    return static_cast<double>(GetExclusiveCount(sfid)) / GetTotalCallstacks();
  }
  [[nodiscard]] uint64_t GetTotalCallstacks() const { return callstack_count_; }

  uint64_t callstack_count_;
  absl::flat_hash_map<SFID, uint64_t> counts_;
};

class MockFrameTrackStats {
 public:
  explicit MockFrameTrackStats(double compute_average_time_ns)
      : compute_average_time_ns_(compute_average_time_ns) {}
  [[nodiscard]] double ComputeAverageTimeNs() const { return compute_average_time_ns_; };

  double compute_average_time_ns_;
};

}  // namespace

namespace orbit_mizar_models {

constexpr size_t kFunctionCount = 4;
constexpr std::array<SFID, kFunctionCount> kSfids = {SFID(1), SFID(2), SFID(3), SFID(4)};
constexpr std::array<uint64_t, kFunctionCount> kBaselineCounts = {0, 0, 4, 3};
constexpr std::array<uint64_t, kFunctionCount> kComparisonCounts = {0, 5, 0, 2};
const std::vector<std::string> kFunctionBaselineNames = {"baseline_foo", "baseline_bar",
                                                         "baseline_biz", "baseline_fiz"};
const std::vector<std::string> kFunctionComparisonNames = {"comparison_foo", "comparison_bar",
                                                           "comparison_biz", "comparison_fiz"};

const std::vector<std::string> kBaselineModules = {"baseline_M1", "baseline_M1", "baseline_M2",
                                                   "baseline_M2"};
const std::vector<std::string> kComparisonModules = {"comparison_M1", "comparison_M1",
                                                     "comparison_M2", "comparison_M2"};

const std::vector<BaselineAndComparisonFunctionSymbols> kSymbols = [] {
  std::vector<BaselineAndComparisonFunctionSymbols> result;
  for (size_t i = 0; i < kFunctionCount; ++i) {
    auto baseline_symbol =
        MakeBaseline<FunctionSymbol>(kFunctionBaselineNames[i], kBaselineModules[i]);
    auto comparison_symbol =
        MakeComparison<FunctionSymbol>(kFunctionComparisonNames[i], kComparisonModules[i]);
    BaselineAndComparisonFunctionSymbols symbols{baseline_symbol, comparison_symbol};
    result.emplace_back(symbols);
  }
  return result;
}();

orbit_mizar_data::CorrectedComparisonResult res = {{0, 1}, 1};
constexpr std::array<orbit_mizar_data::CorrectedComparisonResult, kFunctionCount>
    kComparisonResults = {orbit_mizar_data::CorrectedComparisonResult{{0, 0.1}, 0.2},
                          orbit_mizar_data::CorrectedComparisonResult{{0, 0.01}, 0.02},
                          orbit_mizar_data::CorrectedComparisonResult{{0, 0.001}, 0.002},
                          orbit_mizar_data::CorrectedComparisonResult{{0, 1}, 1}};
constexpr int kExpectedReportSize = 3;

const absl::flat_hash_map<SFID, uint64_t> kSfidToBaselineCounts = MakeMap(kSfids, kBaselineCounts);
const absl::flat_hash_map<SFID, uint64_t> kSfidToComparisonCounts =
    MakeMap(kSfids, kComparisonCounts);
const absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols> kSfidToBaselineName =
    MakeMap(kSfids, kSymbols);
const absl::flat_hash_map<std::string, SFID> kBaselineNameToSfid =
    MakeMap(kFunctionBaselineNames, kSfids);
const absl::flat_hash_map<std::string, SFID> kComparisonNameToSfid =
    MakeMap(kFunctionComparisonNames, kSfids);
const absl::flat_hash_map<SFID, orbit_mizar_data::CorrectedComparisonResult>
    kSfidToComparisonResult = MakeMap(kSfids, kComparisonResults);

constexpr uint64_t kCallstacksCount = 15;

constexpr uint64_t kBaselineFrameTime = 15'000'000;
constexpr uint64_t kComparisonFrameTime = 25'000'000;

static double ExpectDoubleAndParse(const QString& str) {
  bool ok;
  double result = str.toDouble(&ok);
  EXPECT_TRUE(ok);
  return result;
}

constexpr double kTolerance = 1e-3;

static void ExpectNumericDisplayEq(const QString& actual, double expected) {
  EXPECT_THAT(ExpectDoubleAndParse(actual), DoubleNear(expected, kTolerance));
}

class SamplingWithFrameTrackReportModelTest : public ::testing::Test {
 public:
  using Report =
      orbit_mizar_data::SamplingWithFrameTrackComparisonReportTmpl<MockCounts, MockFrameTrackStats>;
  using Model = SamplingWithFrameTrackReportModelTmpl<Report, MockCounts, MockFrameTrackStats>;

  using Column = Model::Column;

  SamplingWithFrameTrackReportModelTest() = default;

  Baseline<MockCounts> baseline_counts_ =
      MakeBaseline<MockCounts>(kCallstacksCount, kSfidToBaselineCounts);
  Comparison<MockCounts> comparison_counts_ =
      MakeComparison<MockCounts>(kCallstacksCount, kSfidToComparisonCounts);

  Baseline<MockFrameTrackStats> baseline_frame_track_stats_ =
      MakeBaseline<MockFrameTrackStats>(static_cast<double>(kBaselineFrameTime));
  Comparison<MockFrameTrackStats> comparison_frame_track_stats_ =
      MakeComparison<MockFrameTrackStats>(static_cast<double>(kComparisonFrameTime));

  [[nodiscard]] QString DisplayedString(int row, Column column) const {
    return model_.data(MakeIndex(row, column), Qt::DisplayRole).toString();
  }

  [[nodiscard]] QVariant SortValue(int row, Column column) const {
    return model_.data(MakeIndex(row, column), Qt::EditRole);
  }

  [[nodiscard]] QVariant ToolTipValue(int row, Column column) const {
    return model_.data(MakeIndex(row, column), Qt::ToolTipRole);
  }

  [[nodiscard]] QModelIndex MakeIndex(int row, Column column) const {
    return model_.index(row, static_cast<int>(column));
  }

  void ExpectDataChangeIsCorrectlyEmitted(
      const QList<QVariant>& arguments,
      SamplingWithFrameTrackReportModelTest::Column column) const {
    const QModelIndex top_left = arguments.at(0).toModelIndex();
    EXPECT_EQ(top_left.row(), 0);
    EXPECT_EQ(top_left.column(), static_cast<int>(column));

    const QModelIndex bottom_right = arguments.at(1).toModelIndex();
    EXPECT_EQ(bottom_right.row(), model_.rowCount({}) - 1);
    EXPECT_EQ(bottom_right.column(), static_cast<int>(column));
  }

  void ExpectPvalueAndIsSignificantAreCorrect(const bool is_multiplicity_correction_enabled,
                                              const double significance_level) {
    model_.SetMultiplicityCorrectionEnabled(is_multiplicity_correction_enabled);
    model_.SetSignificanceLevel(significance_level);

    for (int row = 0; row < model_.rowCount({}); ++row) {
      const QString name = DisplayedString(row, Column::kFunctionName);

      ASSERT_THAT(kFunctionBaselineNames, Contains(name.toStdString()));
      const SFID sfid = kBaselineNameToSfid.at(name.toStdString());

      const orbit_mizar_data::CorrectedComparisonResult& comparison_result =
          kSfidToComparisonResult.at(sfid);

      const double expected_pvalue = is_multiplicity_correction_enabled
                                         ? comparison_result.corrected_pvalue
                                         : comparison_result.pvalue;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(row, Column::kPvalue,
                                                                      expected_pvalue);

      const std::string expected_is_significant =
          expected_pvalue < significance_level ? "Yes" : "No";
      EXPECT_EQ(DisplayedString(row, Column::kIsSignificant).toStdString(),
                expected_is_significant);

      const QVariant is_significant_sort_value = SortValue(row, Column::kIsSignificant);
      EXPECT_EQ(is_significant_sort_value.type(), QMetaType::Double);
      EXPECT_THAT(is_significant_sort_value.value<double>(), expected_pvalue);
    }
  }

  void ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(const int row,
                                                                       const Column column,
                                                                       double expected) const {
    const QString displayed = DisplayedString(row, column);
    ExpectNumericDisplayEq(displayed, expected);
    const QVariant sort_value = SortValue(row, column);
    EXPECT_EQ(sort_value.type(), QMetaType::Double);
    EXPECT_THAT(sort_value.value<double>(), DoubleNear(expected, kTolerance));
    const QVariant tool_tip_value = ToolTipValue(row, column);
    EXPECT_EQ(tool_tip_value.type(), QMetaType::QString);
    EXPECT_THAT(tool_tip_value.value<QString>().toStdString(), StrNe(""));
  }

  Report report_{baseline_counts_,        baseline_frame_track_stats_,
                 comparison_counts_,      comparison_frame_track_stats_,
                 kSfidToComparisonResult, &kSfidToBaselineName};

  Model model_{report_, /*is_multiplicity_correction_enabled=*/true, /*significance_level=*/0.05,
               Model::FunctionNameToShow::kBaseline};
};

using FunctionNameToShow = SamplingWithFrameTrackReportModelTest::Model::FunctionNameToShow;

[[nodiscard]] static std::optional<SFID> DisplayedNameToSfid(
    const absl::flat_hash_map<std::string, SFID>& name_to_sfid, const QString& name) {
  if (const auto it = name_to_sfid.find(name.toStdString()); it != name_to_sfid.end()) {
    return it->second;
  }
  return std::nullopt;
}

[[nodiscard]] static std::optional<SFID> DisplayedNameToSfid(
    FunctionNameToShow function_name_to_show, const QString& actual_name) {
  switch (function_name_to_show) {
    case FunctionNameToShow::kBaseline:
      return DisplayedNameToSfid(kBaselineNameToSfid, actual_name);
    case FunctionNameToShow::kComparison:
      return DisplayedNameToSfid(kComparisonNameToSfid, actual_name);
    default:
      ORBIT_UNREACHABLE();
  }
}

TEST_F(SamplingWithFrameTrackReportModelTest, DisplayedDataIsCorrect) {
  EXPECT_EQ(model_.rowCount({}), kExpectedReportSize);

  absl::flat_hash_set<SFID> observed_sfids;

  for (FunctionNameToShow function_name_to_show :
       {FunctionNameToShow::kBaseline, FunctionNameToShow::kComparison}) {
    for (int row = 0; row < model_.rowCount({}); ++row) {
      model_.SetFunctionNameToShow(function_name_to_show);
      const QString name = DisplayedString(row, Column::kFunctionName);
      const std::optional<SFID> optional_sfid = DisplayedNameToSfid(function_name_to_show, name);

      ASSERT_THAT(optional_sfid, Ne(std::nullopt));
      const SFID sfid = *optional_sfid;

      EXPECT_EQ(name.toLower(), SortValue(row, Column::kFunctionName));

      observed_sfids.insert(sfid);
      const uint64_t expected_baseline_count = kSfidToBaselineCounts.at(sfid);
      const uint64_t expected_comparison_count = kSfidToComparisonCounts.at(sfid);

      const double expected_baseline_exclusive_percent =
          static_cast<double>(expected_baseline_count) / kCallstacksCount * 100;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(
          row, Column::kBaselineExclusivePercent, expected_baseline_exclusive_percent);

      constexpr double kNsInUs = 1'000;

      const double expected_baseline_time_per_frame = static_cast<double>(expected_baseline_count) /
                                                      kCallstacksCount * kBaselineFrameTime /
                                                      kNsInUs;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(
          row, Column::kBaselineExclusiveTimePerFrame, expected_baseline_time_per_frame);

      const double expected_comparison_exclusive_percent =
          static_cast<double>(expected_comparison_count) / kCallstacksCount * 100;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(
          row, Column::kComparisonExclusivePercent, expected_comparison_exclusive_percent);

      const double expected_comparison_time_per_frame =
          static_cast<double>(expected_comparison_count) / kCallstacksCount * kComparisonFrameTime /
          kNsInUs;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(
          row, Column::kComparisonExclusiveTimePerFrame, expected_comparison_time_per_frame);

      const double expected_slowdown =
          expected_comparison_time_per_frame - expected_baseline_time_per_frame;
      const double expected_slowdown_percent =
          expected_slowdown / expected_baseline_time_per_frame * 100;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(row, Column::kSlowdownPercent,
                                                                      expected_slowdown_percent);

      const double frame_time_slowdown = (kComparisonFrameTime - kBaselineFrameTime) / kNsInUs;
      const double expected_percent_of_slowdown = expected_slowdown / frame_time_slowdown * 100;
      ExpectNumericDisplayAndSortValuesAreCorrectAndAToolTipIsPresent(
          row, Column::kPercentOfSlowdown, expected_percent_of_slowdown);
    }
  }

  EXPECT_EQ(observed_sfids.size(), kExpectedReportSize);
}

TEST_F(SamplingWithFrameTrackReportModelTest,
       SignificanceLevelAndMultiplicityCorrectionUpdateTriggersSignal) {
  QSignalSpy spy(&model_, &SamplingWithFrameTrackReportModel::dataChanged);

  model_.SetSignificanceLevel(0.01);
  model_.SetMultiplicityCorrectionEnabled(false);

  EXPECT_EQ(spy.count(), 2);

  ExpectDataChangeIsCorrectlyEmitted(spy.takeFirst(), Column::kIsSignificant);
  ExpectDataChangeIsCorrectlyEmitted(spy.takeLast(), Column::kPvalue);
}

TEST_F(SamplingWithFrameTrackReportModelTest, PvalueAndIsSignificantAreCorrect) {
  ExpectPvalueAndIsSignificantAreCorrect(/*is_multiplicity_correction_enabled=*/true,
                                         /*significance_level=*/0.05);
  ExpectPvalueAndIsSignificantAreCorrect(/*is_multiplicity_correction_enabled=*/false,
                                         /*significance_level=*/0.05);
  ExpectPvalueAndIsSignificantAreCorrect(/*is_multiplicity_correction_enabled=*/true,
                                         /*significance_level=*/0.01);
  ExpectPvalueAndIsSignificantAreCorrect(/*is_multiplicity_correction_enabled=*/false,
                                         /*significance_level=*/0.01);
}

}  // namespace orbit_mizar_models