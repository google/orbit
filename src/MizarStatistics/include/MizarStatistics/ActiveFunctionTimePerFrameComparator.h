// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_STATISTICS_ACTIVE_FUNCTION_TIME_PER_FRAME_COMPARATOR_H_
#define MIZAR_STATISTICS_ACTIVE_FUNCTION_TIME_PER_FRAME_COMPARATOR_H_

#include <utility>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/SampledFunctionId.h"
#include "Statistics/Gaussian.h"
#include "Statistics/StatisticsUtils.h"

namespace orbit_mizar_statistics {

// Whatever is usually referred to as "Statistical Test" we call "Comparison" in the project to save
// confusion with Unit tests.
struct ComparisonResult {
  double statistic{};

  // The term from Statistics. TL;DR: The smaller it is, the less we believe in the assumption under
  // test (e.g. no difference in active function time).
  double pvalue{};
};

// Implements the statistical hypothesis testing procedure aimed to test the equality of total
// CPU-time of the sampled functions given the sampled rates and the frame track stats.
// Under the assumption of equality the distribution of the statistic is approximated with normal
// distribution.
// See "Sampling Data without timestamps + Frametrack" section in go/orbit-comparison-tool-design
// for more detail.
template <typename Counts, typename FrameTrackStats>
class ActiveFunctionTimePerFrameComparatorTmpl {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using SFID = ::orbit_mizar_base::SampledFunctionId;

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
    const Baseline<orbit_statistics::MeanAndVariance> baseline_active_time = LiftAndApply(
        ActiveFunctionTime, baseline_counts_, baseline_frame_stats_, Baseline<SFID>(sfid));
    const Comparison<orbit_statistics::MeanAndVariance> comparison_active_time = LiftAndApply(
        ActiveFunctionTime, comparison_counts_, comparison_frame_stats_, Comparison<SFID>(sfid));
    const orbit_statistics::MeanAndVariance non_normalized_statistic =
        NonNormalizedStatistic(baseline_active_time, comparison_active_time);

    const double statistic =
        non_normalized_statistic.mean / std::sqrt(non_normalized_statistic.variance);

    const double pvalue_right_tail = orbit_statistics::GaussianCdf(statistic);
    if (std::isnan(pvalue_right_tail)) return {statistic, 1.0};

    // But the test is two-tailed. So by the symmetry of Normal distribution we have
    const double pvalue = std::min(pvalue_right_tail, 1 - pvalue_right_tail) * 2;
    return {statistic, pvalue};
  }

 private:
  [[nodiscard]] static orbit_statistics::MeanAndVariance NonNormalizedStatistic(
      const Baseline<orbit_statistics::MeanAndVariance>& baseline_active_time,
      const Comparison<orbit_statistics::MeanAndVariance>& comparison_active_time) {
    return orbit_statistics::DiffOfTwoIndependent(*baseline_active_time, *comparison_active_time);
  }

  [[nodiscard]] static orbit_statistics::MeanAndVariance ActiveFunctionTime(
      const Counts& counts, const FrameTrackStats& frame_track_stats, SFID sfid) {
    const double rate = counts.GetExclusiveRate(sfid);
    const double frametime = frame_track_stats.ComputeAverageTimeNs();

    const double rate_variance = rate * (1 - rate) / counts.GetTotalCallstacks();
    const double frametime_variance =
        frame_track_stats.variance_ns() / std::sqrt(frame_track_stats.count());

    return orbit_statistics::ProductOfTwoIndependent({rate, rate_variance},
                                                     {frametime, frametime_variance});
  }

  const Baseline<Counts>& baseline_counts_;
  const Baseline<FrameTrackStats>& baseline_frame_stats_;

  const Comparison<Counts>& comparison_counts_;
  const Comparison<FrameTrackStats>& comparison_frame_stats_;
};

}  // namespace orbit_mizar_statistics

#endif  // MIZAR_STATISTICS_ACTIVE_FUNCTION_TIME_PER_FRAME_COMPARATOR_H_
