// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

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
constexpr int kExpectedReportSize = 3;

const absl::flat_hash_map<SFID, uint64_t> kSfidToBaselineCounts = MakeMap(kSfids, kBaselineCounts);
const absl::flat_hash_map<SFID, uint64_t> kSfidToComparisonCounts =
    MakeMap(kSfids, kComparisonCounts);
const absl::flat_hash_map<SFID, std::string> kSfidToName = MakeMap(kSfids, kFunctionNames);
const absl::flat_hash_map<std::string, SFID> kNameToSfid = MakeMap(kFunctionNames, kSfids);

constexpr uint64_t kCallstacksCount = 15;

constexpr uint64_t kBaselineFrameTime = 15'000'000;
constexpr uint64_t kComparisonFrameTime = 25'000'000;

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

  Report report_{baseline_counts_,
                 baseline_frame_track_stats_,
                 comparison_counts_,
                 comparison_frame_track_stats_,
                 {},
                 &kSfidToName};

  Model model_{report_};
};

static double ExpectDoubleAndParse(const QString& str) {
  bool ok;
  double result = str.toDouble(&ok);
  EXPECT_TRUE(ok);
  return result;
}

TEST_F(SamplingWithFrameTrackReportModelTest, DisplayedDataIsCorrect) {
  EXPECT_EQ(model_.rowCount({}), kExpectedReportSize);

  absl::flat_hash_set<SFID> observed_sfids;

  for (int row = 0; row < model_.rowCount({}); ++row) {
    const QString name = DisplayedString(row, Column::kFunctionName);

    ASSERT_THAT(kFunctionNames, testing::Contains(name.toStdString()));
    const SFID sfid = kNameToSfid.at(name.toStdString());
    observed_sfids.insert(sfid);
    const uint64_t expected_baseline_count = kSfidToBaselineCounts.at(sfid);
    const uint64_t expected_comparison_count = kSfidToComparisonCounts.at(sfid);

    constexpr double kTolerance = 1e-3;

    const QString baseline_exclusive_percent =
        DisplayedString(row, Column::kBaselineExclusivePercent);
    EXPECT_THAT(ExpectDoubleAndParse(baseline_exclusive_percent),
                DoubleNear(static_cast<double>(expected_baseline_count) / kCallstacksCount * 100,
                           kTolerance));

    constexpr double kNsInUs = 1'000;

    const QString baseline_exclusive_per_frame =
        DisplayedString(row, Column::kBaselineExclusiveTimePerFrame);
    EXPECT_THAT(ExpectDoubleAndParse(baseline_exclusive_per_frame),
                DoubleNear(static_cast<double>(expected_baseline_count) / kCallstacksCount *
                               kBaselineFrameTime / kNsInUs,
                           kTolerance));

    const QString comparison_exclusive_percent =
        DisplayedString(row, Column::kComparisonExclusivePercent);
    EXPECT_THAT(ExpectDoubleAndParse(comparison_exclusive_percent),
                DoubleNear(static_cast<double>(expected_comparison_count) / kCallstacksCount * 100,
                           kTolerance));
    const QString comparison_exclusive_per_frame =
        DisplayedString(row, Column::kComparisonExclusiveTimePerFrame);
    EXPECT_THAT(ExpectDoubleAndParse(comparison_exclusive_per_frame),
                DoubleNear(static_cast<double>(expected_comparison_count) / kCallstacksCount *
                               kComparisonFrameTime / kNsInUs,
                           kTolerance));
  }

  EXPECT_EQ(observed_sfids.size(), kExpectedReportSize);
}

}  // namespace orbit_mizar_widgets