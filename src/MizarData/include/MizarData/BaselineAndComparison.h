// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/ScopeStats.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/MizarDataProvider.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarStatistics/ActiveFunctionTimePerFrameComparator.h"
#include "Statistics/MultiplicityCorrection.h"

namespace orbit_mizar_data {

// The class owns the data from two capture files via owning two instances of
// `PairedData`. Also owns the map from sampled function ids to the
// corresponding function names.
template <typename PairedData, typename FunctionTimeComparator, auto MultiplicityCorrection>
class BaselineAndComparisonTmpl {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using SFID = ::orbit_mizar_base::SampledFunctionId;
  using TID = ::orbit_mizar_base::TID;
  using RelativeTimeNs = ::orbit_mizar_base::RelativeTimeNs;
  using BaselineAndComparisonFunctionSymbols =
      ::orbit_mizar_base::BaselineAndComparisonFunctionSymbols;

 public:
  BaselineAndComparisonTmpl(
      Baseline<PairedData> baseline, Comparison<PairedData> comparison,
      absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols> sfid_to_symbols)
      : baseline_(std::move(baseline)),
        comparison_(std::move(comparison)),
        sfid_to_symbols_(std::move(sfid_to_symbols)) {}

  [[nodiscard]] const absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols>&
  sfid_to_symbols() const {
    return sfid_to_symbols_;
  }

  [[nodiscard]] SamplingWithFrameTrackComparisonReport MakeSamplingWithFrameTrackReport(
      Baseline<HalfOfSamplingWithFrameTrackReportConfig> baseline_config,
      Comparison<HalfOfSamplingWithFrameTrackReportConfig> comparison_config) const {
    Baseline<SamplingCounts> baseline_sampling_counts =
        LiftAndApply(MakeCounts, baseline_, baseline_config);
    Baseline<orbit_client_data::ScopeStats> baseline_frame_stats =
        LiftAndApply(MakeFrameTrackStats, baseline_, baseline_config);

    Comparison<SamplingCounts> comparison_sampling_counts =
        LiftAndApply(MakeCounts, comparison_, comparison_config);
    Comparison<orbit_client_data::ScopeStats> comparison_frame_stats =
        LiftAndApply(MakeFrameTrackStats, comparison_, comparison_config);

    FunctionTimeComparator comparator(baseline_sampling_counts, baseline_frame_stats,
                                      comparison_sampling_counts, comparison_frame_stats);

    absl::flat_hash_map<SFID, CorrectedComparisonResult> sfid_to_corrected_comparison_result =
        MakeComparisons(comparator);

    return SamplingWithFrameTrackComparisonReport(
        std::move(baseline_sampling_counts), std::move(baseline_frame_stats),
        std::move(comparison_sampling_counts), std::move(comparison_frame_stats),
        std::move(sfid_to_corrected_comparison_result), &sfid_to_symbols_);
  }

  [[nodiscard]] const Baseline<PairedData>& GetBaselineData() const { return baseline_; }
  [[nodiscard]] const Comparison<PairedData>& GetComparisonData() const { return comparison_; }

 private:
  [[nodiscard]] absl::flat_hash_map<SFID, CorrectedComparisonResult> MakeComparisons(
      const FunctionTimeComparator& comparator) const {
    absl::flat_hash_map<SFID, orbit_mizar_statistics::ComparisonResult> results;
    absl::flat_hash_map<SFID, double> pvalues;
    for (const auto& [sfid, unused_name] : sfid_to_symbols_) {
      orbit_mizar_statistics::ComparisonResult result = comparator.Compare(sfid);
      results.try_emplace(sfid, result);
      pvalues.try_emplace(sfid, result.pvalue);
    }

    const absl::flat_hash_map<SFID, double> corrected_pvalues = MultiplicityCorrection(pvalues);

    absl::flat_hash_map<SFID, CorrectedComparisonResult> corrected;

    for (const auto& [sfid, result] : results) {
      corrected.try_emplace(sfid, CorrectedComparisonResult{result, corrected_pvalues.at(sfid)});
    }
    return corrected;
  }

  [[nodiscard]] static orbit_client_data::ScopeStats MakeFrameTrackStats(
      const PairedData& data, const HalfOfSamplingWithFrameTrackReportConfig& config) {
    return data.ActiveInvocationTimeStats(config.tids, config.frame_track_id, config.start_relative,
                                          config.EndRelative());
  }

  [[nodiscard]] static SamplingCounts MakeCounts(
      const PairedData& data, const HalfOfSamplingWithFrameTrackReportConfig& config) {
    uint64_t total_callstacks = 0;
    absl::flat_hash_map<SFID, InclusiveAndExclusive> counts;
    for (const TID tid : config.tids) {
      data.ForEachCallstackEvent(tid, config.start_relative, config.EndRelative(),
                                 [&total_callstacks, &counts](absl::Span<const SFID> callstack) {
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
  absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols> sfid_to_symbols_;
};

using ActiveFunctionTimePerFrameComparator =
    orbit_mizar_statistics::ActiveFunctionTimePerFrameComparatorTmpl<SamplingCounts,
                                                                     orbit_client_data::ScopeStats>;

using BaselineAndComparison = BaselineAndComparisonTmpl<
    MizarPairedData, ActiveFunctionTimePerFrameComparator,
    orbit_statistics::HolmBonferroniCorrection<orbit_mizar_base::SampledFunctionId>>;

BaselineAndComparison CreateBaselineAndComparison(std::unique_ptr<MizarDataProvider> baseline,
                                                  std::unique_ptr<MizarDataProvider> comparison);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_H_