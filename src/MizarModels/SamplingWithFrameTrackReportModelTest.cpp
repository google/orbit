// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <Qt>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarModels/SamplingWithFrameTrackReportModel.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;
using SFID = ::orbit_mizar_base::SFID;
using Counts = ::orbit_mizar_data::SamplingCounts;
using ::orbit_mizar_base::MakeBaseline;
using ::orbit_mizar_base::MakeComparison;
using ::orbit_test_utils::MakeMap;
using ::testing::DoubleNear;
using ::testing::Invoke;
using ::testing::Return;

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

namespace orbit_mizar_widgets {

constexpr size_t kFunctionCount = 4;
constexpr std::array<SFID, kFunctionCount> kSfids = {SFID(1), SFID(2), SFID(3), SFID(4)};
constexpr std::array<uint64_t, kFunctionCount> kBaselineCounts = {0, 0, 4, 3};
constexpr std::array<uint64_t, kFunctionCount> kComparisonCounts = {0, 5, 0, 2};
const std::vector<std::string> kFunctionNames = {"foo", "bar", "biz", "fiz"};
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
const absl::flat_hash_map<SFID, std::string> kSfidToName = MakeMap(kSfids, kFunctionNames);
const absl::flat_hash_map<std::string, SFID> kNameToSfid = MakeMap(kFunctionNames, kSfids);
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
      MakeBaseline<MockFrameTrackStats>(kBaselineFrameTime);
  Comparison<MockFrameTrackStats> comparison_frame_track_stats_ =
      MakeComparison<MockFrameTrackStats>(kComparisonFrameTime);

  [[nodiscard]] QString DisplayedString(int row, Column column) const {
    const QModelIndex index = model_.index(row, static_cast<int>(column));
    return model_.data(index, Qt::DisplayRole).toString();
  }

  void ExpectDataChangeIsCorrectlyEmitted(
      const QList<QVariant>& arguments,
      SamplingWithFrameTrackReportModelTest::Column column) const {
    const QModelIndex top_left = arguments.at(0).toModelIndex();
    EXPECT_EQ(top_left.row(), 0);
    EXPECT_EQ(top_left.column(), static_cast<int>(column));

    const QModelIndex bottom_right = arguments.at(1).toModelIndex();
    EXPECT_EQ(bottom_right.row(), model_.rowCount() - 1);
    EXPECT_EQ(bottom_right.column(), static_cast<int>(column));
  }

  void ExpectPvalueAndIsSignificantAreCorrect(const bool is_multiplicity_correction_enabled,
                                              const double significance_level) {
    model_.SetMultiplicityCorrectionEnabled(is_multiplicity_correction_enabled);
    model_.SetSignificanceLevel(significance_level);

    for (int row = 0; row < model_.rowCount(); ++row) {
      const QString name = DisplayedString(row, Column::kFunctionName);

      ASSERT_THAT(kFunctionNames, testing::Contains(name.toStdString()));
      const SFID sfid = kNameToSfid.at(name.toStdString());

      const orbit_mizar_data::CorrectedComparisonResult& comparison_result =
          kSfidToComparisonResult.at(sfid);

      const QString pvalue = DisplayedString(row, Column::kPvalue);
      const double expected_pvalue = is_multiplicity_correction_enabled
                                         ? comparison_result.corrected_pvalue
                                         : comparison_result.pvalue;
      const bool expected_is_significant = expected_pvalue < significance_level;
      ExpectNumericDisplayEq(pvalue, expected_pvalue);
      EXPECT_EQ(DisplayedString(row, Column::kIsSignificant).toStdString(),
                expected_is_significant ? "Yes" : "No");
    }
  }

  Report report_{baseline_counts_,        baseline_frame_track_stats_,
                 comparison_counts_,      comparison_frame_track_stats_,
                 kSfidToComparisonResult, &kSfidToName};

  Model model_{report_, true, 0.05};
};

TEST_F(SamplingWithFrameTrackReportModelTest, DisplayedDataIsCorrect) {
  EXPECT_EQ(model_.rowCount(), kExpectedReportSize);

  absl::flat_hash_set<SFID> observed_sfids;

  for (int row = 0; row < model_.rowCount(); ++row) {
    const QString name = DisplayedString(row, Column::kFunctionName);

    ASSERT_THAT(kFunctionNames, testing::Contains(name.toStdString()));
    const SFID sfid = kNameToSfid.at(name.toStdString());
    observed_sfids.insert(sfid);
    const uint64_t expected_baseline_count = kSfidToBaselineCounts.at(sfid);
    const uint64_t expected_comparison_count = kSfidToComparisonCounts.at(sfid);

    const QString baseline_exclusive_percent =
        DisplayedString(row, Column::kBaselineExclusivePercent);
    ExpectNumericDisplayEq(baseline_exclusive_percent,
                           static_cast<double>(expected_baseline_count) / kCallstacksCount * 100);

    constexpr double kNsInUs = 1'000;

    const QString baseline_exclusive_per_frame =
        DisplayedString(row, Column::kBaselineExclusiveTimePerFrame);
    ExpectNumericDisplayEq(baseline_exclusive_per_frame,
                           static_cast<double>(expected_baseline_count) / kCallstacksCount *
                               kBaselineFrameTime / kNsInUs);

    const QString comparison_exclusive_percent =
        DisplayedString(row, Column::kComparisonExclusivePercent);
    ExpectNumericDisplayEq(comparison_exclusive_percent,
                           static_cast<double>(expected_comparison_count) / kCallstacksCount * 100);
    const QString comparison_exclusive_per_frame =
        DisplayedString(row, Column::kComparisonExclusiveTimePerFrame);
    ExpectNumericDisplayEq(comparison_exclusive_per_frame,
                           static_cast<double>(expected_comparison_count) / kCallstacksCount *
                               kComparisonFrameTime / kNsInUs);
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

}  // namespace orbit_mizar_widgets