// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>

#include <string>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/ActiveFunctionTimePerFrameComparator.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/NonWrappingAddition.h"
#include "MizarData/SampledFunctionId.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"

namespace orbit_mizar_data {

// The class owns the data from two capture files via owning two instances of
// `PairedData`. Also owns the map from sampled function ids to the
// corresponding function names.
template <typename PairedData, typename FunctionTimeComparator>
class BaselineAndComparisonTmpl {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;

 public:
  BaselineAndComparisonTmpl(Baseline<PairedData> baseline, Comparison<PairedData> comparison,
                            absl::flat_hash_map<SFID, std::string> sfid_to_name)
      : baseline_(std::move(baseline)),
        comparison_(std::move(comparison)),
        sfid_to_name_(std::move(sfid_to_name)) {}

  [[nodiscard]] const absl::flat_hash_map<SFID, std::string>& sfid_to_name() const {
    return sfid_to_name_;
  }

  [[nodiscard]] SamplingWithFrameTrackComparisonReport MakeSamplingWithFrameTrackReport(
      Baseline<HalfOfSamplingWithFrameTrackReportConfig> baseline_config,
      Comparison<HalfOfSamplingWithFrameTrackReportConfig> comparison_config) const {
    Baseline<SamplingCounts> baseline_sampling_counts =
        Call(MakeCounts, baseline_, baseline_config);
    Baseline<orbit_client_data::ScopeStats> baseline_frame_stats =
        Call(MakeFrameTrackStats, baseline_, baseline_config);

    Comparison<SamplingCounts> comparison_sampling_counts =
        Call(MakeCounts, comparison_, comparison_config);
    Comparison<orbit_client_data::ScopeStats> comparison_frame_stats =
        Call(MakeFrameTrackStats, comparison_, comparison_config);

    FunctionTimeComparator comparator(baseline_sampling_counts, baseline_frame_stats,
                                      comparison_sampling_counts, comparison_frame_stats);

    absl::flat_hash_map<SFID, ComparisonResult> sfid_to_comparison_result;
    for (const auto& [sfid, unused_name] : sfid_to_name_) {
      sfid_to_comparison_result.try_emplace(sfid, comparator.Compare(sfid));
    }

    return SamplingWithFrameTrackComparisonReport(
        std::move(baseline_sampling_counts), std::move(baseline_frame_stats),
        std::move(comparison_sampling_counts), std::move(comparison_frame_stats),
        std::move(sfid_to_comparison_result));
  }

 private:
  [[nodiscard]] static orbit_client_data::ScopeStats MakeFrameTrackStats(
      const PairedData& data, const HalfOfSamplingWithFrameTrackReportConfig& config) {
    const std::vector<uint64_t> active_invocation_times = data.ActiveInvocationTimes(
        config.tids, config.frame_track_scope_id, config.start_relative_ns,
        NonWrappingAddition(config.start_relative_ns, config.duration_ns));
    orbit_client_data::ScopeStats stats;
    for (const uint64_t active_invocation_time : active_invocation_times) {
      stats.UpdateStats(active_invocation_time);
    }
    return stats;
  }

  [[nodiscard]] static SamplingCounts MakeCounts(
      const PairedData& data, const HalfOfSamplingWithFrameTrackReportConfig& config) {
    uint64_t total_callstacks = 0;
    absl::flat_hash_map<SFID, InclusiveAndExclusive> counts;
    for (const uint32_t tid : config.tids) {
      data.ForEachCallstackEvent(tid, config.start_relative_ns,
                                 NonWrappingAddition(config.start_relative_ns, config.duration_ns),
                                 [&total_callstacks, &counts](const std::vector<SFID>& callstack) {
                                   total_callstacks++;
                                   if (callstack.empty()) return;
                                   for (const SFID sfid : callstack) {
                                     counts[sfid].inclusive++;
                                   }
                                   counts[callstack.front()].exclusive++;
                                 });
    }

    return SamplingCounts(std::move(counts), total_callstacks);
  }

  Baseline<PairedData> baseline_;
  Comparison<PairedData> comparison_;
  absl::flat_hash_map<SFID, std::string> sfid_to_name_;
};

using BaselineAndComparison =
    BaselineAndComparisonTmpl<MizarPairedData, ActiveFunctionTimePerFrameComparator>;

BaselineAndComparison CreateBaselineAndComparison(std::unique_ptr<MizarDataProvider> baseline,
                                                  std::unique_ptr<MizarDataProvider> comparison);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_H_