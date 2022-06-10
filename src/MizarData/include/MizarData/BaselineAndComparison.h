// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_H_

#include <stdint.h>

#include <string>

#include "ClientData/CallstackData.h"
#include "ClientData/CaptureData.h"
#include "MizarData/MizarData.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/NonWrappingAddition.h"
#include "MizarData/SampledFunctionId.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"

namespace orbit_mizar_data {

// The class owns the data from two capture files via owning two instances of
// `PairedData`. Also owns the map from sampled function ids to the
// corresponding function names.
template <typename PairedData>
class BaselineAndComparisonTmpl {
 public:
  BaselineAndComparisonTmpl(PairedData baseline, PairedData comparison,
                            absl::flat_hash_map<SFID, std::string> sfid_to_name)
      : baseline_(std::move(baseline)),
        comparison_(std::move(comparison)),
        sfid_to_name_(std::move(sfid_to_name)) {}

  [[nodiscard]] const absl::flat_hash_map<SFID, std::string>& sfid_to_name() const {
    return sfid_to_name_;
  }

  [[nodiscard]] SamplingWithFrameTrackComparisonReport MakeSamplingWithFrameTrackReport(
      BaselineSamplingWithFrameTrackReportConfig baseline_config,
      ComparisonSamplingWithFrameTrackReportConfig comparison_config) const {
    return {MakeCounts(baseline_, baseline_config), MakeCounts(comparison_, comparison_config),
            MakeFrameTrackStats(baseline_, comparison_config),
            MakeFrameTrackStats(comparison_, comparison_config)};
  }

 private:
  [[nodiscard]] orbit_client_data::ScopeStats MakeFrameTrackStats(
      const PairedData& data, const HalfOfSamplingWithFrameTrackReportConfig& config) const {
    const std::vector<uint64_t> active_invocation_times = data.ActiveInvocationTimes(
        config.frame_track_scope_id, config.start_relative_ns,
        NonWrappingAddition(config.start_relative_ns, config.duration_ns));
    orbit_client_data::ScopeStats stats;
    for (const uint64_t active_invocation_time : active_invocation_times) {
      stats.UpdateStats(active_invocation_time);
    }
    return stats;
  }

  [[nodiscard]] SamplingCounts MakeCounts(
      const PairedData& data, const HalfOfSamplingWithFrameTrackReportConfig& config) const {
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
                                   counts[callstack.back()].exclusive++;
                                 });
    }
    return SamplingCounts(std::move(counts), total_callstacks);
  }

  PairedData baseline_;
  PairedData comparison_;
  absl::flat_hash_map<SFID, std::string> sfid_to_name_;
};

using BaselineAndComparison = BaselineAndComparisonTmpl<MizarPairedData<MizarDataProvider>>;

BaselineAndComparison CreateBaselineAndComparison(std::unique_ptr<MizarDataProvider> baseline,
                                                  std::unique_ptr<MizarDataProvider> comparison);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_H_