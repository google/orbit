// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <utility>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarStatistics/ActiveFunctionTimePerFrameComparator.h"

using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::SampledFunctionId;
using ::testing::DoubleNear;
using ::testing::IsNan;
using ::testing::Return;

namespace {
class MockCounts {
 public:
  MOCK_METHOD(double, GetExclusiveRate, (SampledFunctionId), (const));
  MOCK_METHOD(uint64_t, GetTotalCallstacks, (), (const));
};

class MockFrameTrackStats {
 public:
  MOCK_METHOD(double, ComputeAverageTimeNs, (), (const));
  MOCK_METHOD(uint64_t, variance_ns, (), (const));
  MOCK_METHOD(uint64_t, count, (), (const));
};

}  // namespace

namespace orbit_mizar_statistics {

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

constexpr SampledFunctionId kArbitrarySfid(10);

constexpr double kExpectedStatistic = -0.929944;
constexpr double kPvalue = 0.352400;
constexpr double kTolerance = 1e-3;

class ActiveFunctionTimePerFrameComparatorTest : public ::testing::Test {
 public:
  ActiveFunctionTimePerFrameComparatorTest()
      : baseline_counts_(std::in_place),
        comparison_counts_(std::in_place),
        baseline_frame_track_stats_(std::in_place),
        comparison_frame_track_stats_(std::in_place) {}

 protected:
  Baseline<MockCounts> baseline_counts_;
  Comparison<MockCounts> comparison_counts_;

  Baseline<MockFrameTrackStats> baseline_frame_track_stats_;
  Comparison<MockFrameTrackStats> comparison_frame_track_stats_;
};

static void SetBaselineExpectations(MockCounts& counts, MockFrameTrackStats& frame_track_stas) {
  EXPECT_CALL(frame_track_stas, ComputeAverageTimeNs).WillRepeatedly(Return(kAvgFrametimeBaseline));
  EXPECT_CALL(frame_track_stas, count).WillRepeatedly(Return(kFramesCountBaseline));
  EXPECT_CALL(frame_track_stas, variance_ns).WillRepeatedly(Return(kFrametimeVarBaseline));
  EXPECT_CALL(counts, GetTotalCallstacks).WillRepeatedly(Return(kTotalCallstacksBaseline));
  EXPECT_CALL(counts, GetExclusiveRate).WillRepeatedly(Return(kRateBaseline));
}

static void SetComparisonExpectations(MockCounts& counts, MockFrameTrackStats& frame_track_stas) {
  EXPECT_CALL(frame_track_stas, ComputeAverageTimeNs)
      .WillRepeatedly(Return(kAvgFrametimeComparison));
  EXPECT_CALL(frame_track_stas, count).WillRepeatedly(Return(kFramesCountComparison));
  EXPECT_CALL(frame_track_stas, variance_ns).WillRepeatedly(Return(kFrametimeVarComparison));
  EXPECT_CALL(counts, GetTotalCallstacks).WillRepeatedly(Return(kTotalCallstacksComparison));
  EXPECT_CALL(counts, GetExclusiveRate).WillRepeatedly(Return(kRateComparison));
}

TEST_F(ActiveFunctionTimePerFrameComparatorTest, ComparatorIsCorrectWithNonZeroRates) {
  ActiveFunctionTimePerFrameComparatorTmpl<MockCounts, MockFrameTrackStats> comparator(
      baseline_counts_, baseline_frame_track_stats_, comparison_counts_,
      comparison_frame_track_stats_);
  SetBaselineExpectations(*baseline_counts_, *baseline_frame_track_stats_);
  SetComparisonExpectations(*comparison_counts_, *comparison_frame_track_stats_);

  ComparisonResult result = comparator.Compare(SampledFunctionId(kArbitrarySfid));

  EXPECT_THAT(result.statistic, DoubleNear(kExpectedStatistic, kTolerance));
  EXPECT_THAT(result.pvalue, DoubleNear(kPvalue, kTolerance));
}

TEST_F(ActiveFunctionTimePerFrameComparatorTest,
       ComparatorIsCorrectWithEqualRatesAndFrameTrackStats) {
  ActiveFunctionTimePerFrameComparatorTmpl<MockCounts, MockFrameTrackStats> comparator(
      baseline_counts_, baseline_frame_track_stats_, comparison_counts_,
      comparison_frame_track_stats_);
  SetBaselineExpectations(*baseline_counts_, *baseline_frame_track_stats_);
  SetBaselineExpectations(*comparison_counts_, *comparison_frame_track_stats_);

  ComparisonResult result = comparator.Compare(SampledFunctionId(kArbitrarySfid));

  EXPECT_THAT(result.statistic, DoubleNear(0, kTolerance));
  // no difference observed, large pvalue returned
  EXPECT_GE(result.pvalue, 0.1);
}

TEST_F(ActiveFunctionTimePerFrameComparatorTest, ComparatorIsCorrectWithZeroRates) {
  ActiveFunctionTimePerFrameComparatorTmpl<MockCounts, MockFrameTrackStats> comparator(
      baseline_counts_, baseline_frame_track_stats_, comparison_counts_,
      comparison_frame_track_stats_);

  // As if no data is observed
  EXPECT_CALL(*baseline_counts_, GetExclusiveRate).WillRepeatedly(Return(0));
  EXPECT_CALL(*comparison_counts_, GetExclusiveRate).WillRepeatedly(Return(0));

  ComparisonResult result = comparator.Compare(SampledFunctionId(kArbitrarySfid));

  EXPECT_THAT(result.statistic, IsNan());
  // no difference observed, largest pvalue returned
  EXPECT_THAT(result.pvalue, DoubleNear(1.0, kTolerance));
}

}  // namespace orbit_mizar_statistics
