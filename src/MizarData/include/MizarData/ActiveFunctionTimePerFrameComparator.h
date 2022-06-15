// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_STATISTICS_ACTIVE_FUNCTION_TIME_PER_FRAME_COMPARATOR_H_
#define MIZAR_STATISTICS_ACTIVE_FUNCTION_TIME_PER_FRAME_COMPARATOR_H_

#include <utility>

#include "ClientData/ScopeStats.h"
#include "MizarData/BaselineOrComparison.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "Statistics/Gaussian.h"
#include "Statistics/StatisticsUtils.h"

namespace orbit_mizar_data {

// TODO(b/236130122) it should be moved to a new library
template <typename Counts, typename FrameTrackStats>
class ActiveFunctionTimePerFrameComparatorTmpl {
 public:
  explicit ActiveFunctionTimePerFrameComparatorTmpl(
      const Baseline<Counts>& baseline_counts,
      const Baseline<FrameTrackStats>& baseline_frame_stats,
      const Comparison<Counts>& comparison_counts,
      const Comparison<FrameTrackStats>& comparison_frame_stats)
      : baseline_counts_(baseline_counts),
        baseline_frame_stats_(baseline_frame_stats),
        comparison_counts_(comparison_counts),
        comparison_frame_stats_(comparison_frame_stats) {}

  [[nodiscard]] ComparisonResult Compare(SFID sfid) const {
    const auto baseline_active_time =
        ActiveFunctionTime(baseline_counts_, baseline_frame_stats_, sfid);
    const auto comparison_active_time =
        ActiveFunctionTime(comparison_counts_, comparison_frame_stats_, sfid);
    const orbit_statistics::MeanAndVariance non_normalized_stat =
        NonNormalizedStat(baseline_active_time, comparison_active_time);

    const double stat = non_normalized_stat.mean / std::sqrt(non_normalized_stat.variance);

    const double pvalue_right_tail = orbit_statistics::GaussianCDF(stat);
    if (std::isnan(pvalue_right_tail)) return {stat, 1.0};

    // But the test is two-tailed. So by the symmetry of Normal distribution we have
    const double pvalue = std::min(pvalue_right_tail, 1 - pvalue_right_tail) * 2;
    return {stat, pvalue};
  }

 private:
  [[nodiscard]] static orbit_statistics::MeanAndVariance NonNormalizedStat(
      const Baseline<orbit_statistics::MeanAndVariance>& baseline_active_time,
      const Comparison<orbit_statistics::MeanAndVariance>& comparison_active_time) {
    return orbit_statistics::DiffOfTwoIndependent(*baseline_active_time, *comparison_active_time);
  }

  template <template <class> class Wrapper>
  [[nodiscard]] static Wrapper<orbit_statistics::MeanAndVariance> ActiveFunctionTime(
      const Wrapper<Counts>& counts, const Wrapper<FrameTrackStats>& frame_track_stats, SFID sfid) {
    const double rate = counts->GetExclusiveRate(sfid);
    const double frametime = frame_track_stats->ComputeAverageTimeNs();

    const double rate_var = rate * (1 - rate) / counts->GetTotalCallstacks();
    const double frametime_var =
        frame_track_stats->variance_ns() / std::sqrt(frame_track_stats->count());

    return Wrapper<orbit_statistics::MeanAndVariance>(
        orbit_statistics::ProductOfTwoIndependent({rate, rate_var}, {frametime, frametime_var}));
  }

  const Baseline<Counts>& baseline_counts_;
  const Baseline<FrameTrackStats>& baseline_frame_stats_;

  const Comparison<Counts>& comparison_counts_;
  const Comparison<FrameTrackStats>& comparison_frame_stats_;
};

using ActiveFunctionTimePerFrameComparator =
    ActiveFunctionTimePerFrameComparatorTmpl<SamplingCounts, orbit_client_data::ScopeStats>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_STATISTICS_ACTIVE_FUNCTION_TIME_PER_FRAME_COMPARATOR_H_
