// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "MizarData/ActiveFunctionTimePerFrameComparator.h"
#include "MizarData/BaselineOrComparison.h"
#include "MizarData/SampledFunctionId.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"

namespace orbit_mizar_data {

using testing::DoubleNear;
using testing::Return;

namespace {
class MockCounts {
 public:
  MOCK_METHOD(double, GetExclusiveRate, (SFID), (const));
  MOCK_METHOD(uint64_t, GetTotalCallstacks, (), (const));
};

class MockFrameTrackStats {
 public:
  MOCK_METHOD(double, ComputeAverageTimeNs, (), (const));
  MOCK_METHOD(uint64_t, variance_ns, (), (const));
  MOCK_METHOD(uint64_t, count, (), (const));
};

}  // namespace

constexpr uint64_t kTotalCallstacksBaseline = 100;
constexpr uint64_t kTotalCallstacksComparison = 200;

constexpr double kRateBaseline = 0.1;
constexpr double kRateComparison = 0.15;

constexpr uint64_t kFramesCountBaseline = 1000;
constexpr uint64_t kFramesCountComparison = 1300;

constexpr double kFrametimeVarBaseline = 100;
constexpr double kFrametimeVarComparison = 150;

constexpr double kAvgFrametimeBaseline = 1000;
constexpr double kAvgFrametimeComparison = 900;

constexpr SFID kArbitrarySfid(10);

constexpr double kExpectedStatistic = -0.929944;
constexpr double kPvalue = 0.352400;
constexpr double kTol = 1e-3;

class ActiveFunctionTimePerFrameComparatorTest : public ::testing::Test {
 public:
  ActiveFunctionTimePerFrameComparatorTest()
      : baseline_counts_(std::in_place_t{}),
        comparision_counts_(std::in_place_t{}),
        baseline_frame_track_stats_(std::in_place_t{}),
        comparison_frame_track_stats_(std::in_place_t{}) {
    EXPECT_CALL(*baseline_frame_track_stats_, ComputeAverageTimeNs)
        .WillRepeatedly(Return(kAvgFrametimeBaseline));
    EXPECT_CALL(*comparison_frame_track_stats_, ComputeAverageTimeNs)
        .WillRepeatedly(Return(kAvgFrametimeComparison));

    EXPECT_CALL(*baseline_frame_track_stats_, count).WillRepeatedly(Return(kFramesCountBaseline));
    EXPECT_CALL(*comparison_frame_track_stats_, count)
        .WillRepeatedly(Return(kFramesCountComparison));

    EXPECT_CALL(*baseline_frame_track_stats_, variance_ns)
        .WillRepeatedly(Return(kFrametimeVarBaseline));
    EXPECT_CALL(*comparison_frame_track_stats_, variance_ns)
        .WillRepeatedly(Return(kFrametimeVarComparison));

    EXPECT_CALL(*baseline_counts_, GetTotalCallstacks)
        .WillRepeatedly(Return(kTotalCallstacksBaseline));
    EXPECT_CALL(*comparision_counts_, GetTotalCallstacks)
        .WillRepeatedly(Return(kTotalCallstacksComparison));
  }

 protected:
  Baseline<MockCounts> baseline_counts_;
  Comparison<MockCounts> comparision_counts_;

  Baseline<MockFrameTrackStats> baseline_frame_track_stats_;
  Comparison<MockFrameTrackStats> comparison_frame_track_stats_;
};

TEST_F(ActiveFunctionTimePerFrameComparatorTest, ComparatorIsCorrectWithNonZeroRates) {
  ActiveFunctionTimePerFrameComparatorTmpl<MockCounts, MockFrameTrackStats> comparator(
      baseline_counts_, baseline_frame_track_stats_, comparision_counts_,
      comparison_frame_track_stats_);
  EXPECT_CALL(*baseline_counts_, GetExclusiveRate).WillRepeatedly(Return(kRateBaseline));
  EXPECT_CALL(*comparision_counts_, GetExclusiveRate).WillRepeatedly(Return(kRateComparison));

  auto result = comparator.Compare(SFID(kArbitrarySfid));

  EXPECT_THAT(result.statistic, DoubleNear(kExpectedStatistic, kTol));
  EXPECT_THAT(result.pvalue, DoubleNear(kPvalue, kTol));
}

TEST_F(ActiveFunctionTimePerFrameComparatorTest, ComparatorIsCorrectWithZeroRates) {
  ActiveFunctionTimePerFrameComparatorTmpl<MockCounts, MockFrameTrackStats> comparator(
      baseline_counts_, baseline_frame_track_stats_, comparision_counts_,
      comparison_frame_track_stats_);
  // As if no data is observed
  EXPECT_CALL(*baseline_counts_, GetExclusiveRate).WillRepeatedly(Return(0));
  EXPECT_CALL(*comparision_counts_, GetExclusiveRate).WillRepeatedly(Return(0));

  ComparisonResult result = comparator.Compare(SFID(kArbitrarySfid));

  EXPECT_THAT(result.statistic, testing::IsNan());
  EXPECT_THAT(result.pvalue,
              DoubleNear(1.0, kTol));  // no difference observed, largest pvalue returned
}

}  // namespace orbit_mizar_data
