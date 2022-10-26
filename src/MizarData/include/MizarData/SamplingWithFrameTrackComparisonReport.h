// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_
#define MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <string>
#include <utility>

#include "ClientData/ScopeStats.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarStatistics/ActiveFunctionTimePerFrameComparator.h"

namespace orbit_mizar_data {

struct CorrectedComparisonResult : public orbit_mizar_statistics::ComparisonResult {
  // result of multiplicity correction (a term from Statistics) for the particular comparison.
  double corrected_pvalue{};
};

// The struct represents the part of configuration relevant to one of the two captures under
// comparison.
class HalfOfSamplingWithFrameTrackReportConfig {
  using TID = ::orbit_mizar_base::TID;
  using FrameTrackId = ::orbit_mizar_data::FrameTrackId;
  using RelativeTimeNs = ::orbit_mizar_base::RelativeTimeNs;

 public:
  explicit HalfOfSamplingWithFrameTrackReportConfig(absl::flat_hash_set<TID> tids,
                                                    RelativeTimeNs start,
                                                    FrameTrackId frame_track_id)
      : tids(std::move(tids)),
        start_relative(start),
        duration(orbit_mizar_base::RelativeTimeNs(std::numeric_limits<uint64_t>::max())),
        frame_track_id(frame_track_id) {}

  [[nodiscard]] RelativeTimeNs EndRelative() const { return Add(start_relative, duration); }

  absl::flat_hash_set<TID> tids{};
  RelativeTimeNs start_relative{};  // nanoseconds elapsed since capture start
  RelativeTimeNs duration{};
  FrameTrackId frame_track_id{};
};

struct InclusiveAndExclusive {
  uint64_t inclusive{};
  uint64_t exclusive{};
};

class SamplingCounts {
  using SFID = ::orbit_mizar_base::SampledFunctionId;

 public:
  explicit SamplingCounts(absl::flat_hash_map<SFID, InclusiveAndExclusive> counts,
                          uint64_t total_callstacks)
      : counts_(std::move(counts)), total_callstacks_(total_callstacks) {}

  [[nodiscard]] uint64_t GetInclusiveCount(SFID sfid) const {
    if (const auto it = counts_.find(sfid); it != counts_.end()) return it->second.inclusive;
    return 0;
  }
  [[nodiscard]] uint64_t GetExclusiveCount(SFID sfid) const {
    if (const auto it = counts_.find(sfid); it != counts_.end()) return it->second.exclusive;
    return 0;
  }

  [[nodiscard]] double GetInclusiveRate(SFID sfid) const {
    if (total_callstacks_ == 0) return 0;
    return static_cast<double>(GetInclusiveCount(sfid)) / GetTotalCallstacks();
  }
  [[nodiscard]] double GetExclusiveRate(SFID sfid) const {
    if (total_callstacks_ == 0) return 0;
    return static_cast<double>(GetExclusiveCount(sfid)) / GetTotalCallstacks();
  }

  [[nodiscard]] uint64_t GetTotalCallstacks() const { return total_callstacks_; }

 private:
  absl::flat_hash_map<SFID, InclusiveAndExclusive> counts_;
  uint64_t total_callstacks_{};
};

// The data class that contains the data we report as results of comparison of the sampling data
// with frame track
template <typename Counts, typename FrameTrackStats>
class SamplingWithFrameTrackComparisonReportTmpl {
 public:
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using SFID = ::orbit_mizar_base::SampledFunctionId;
  using BaselineAndComparisonFunctionSymbols =
      ::orbit_mizar_base::BaselineAndComparisonFunctionSymbols;

  explicit SamplingWithFrameTrackComparisonReportTmpl(
      Baseline<Counts> baseline_sampling_counts,
      Baseline<FrameTrackStats> baseline_frame_track_stats,
      Comparison<Counts> comparison_sampling_counts,
      Comparison<FrameTrackStats> comparison_frame_track_stats,
      absl::flat_hash_map<SFID, CorrectedComparisonResult> fid_to_corrected_comparison_results,
      const absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols>* sfid_to_symbol)
      : baseline_sampling_counts_(std::move(baseline_sampling_counts)),
        baseline_frame_track_stats_(std::move(baseline_frame_track_stats)),
        comparison_sampling_counts_(std::move(comparison_sampling_counts)),
        comparison_frame_track_stats_(std::move(comparison_frame_track_stats)),
        fid_to_corrected_comparison_results_(std::move(fid_to_corrected_comparison_results)),
        sfid_to_symbols_(sfid_to_symbol) {}

  [[nodiscard]] const Baseline<Counts>& GetBaselineSamplingCounts() const {
    return baseline_sampling_counts_;
  }
  [[nodiscard]] const Comparison<Counts>& GetComparisonSamplingCounts() const {
    return comparison_sampling_counts_;
  }

  [[nodiscard]] const Baseline<FrameTrackStats>& GetBaselineFrameTrackStats() const {
    return baseline_frame_track_stats_;
  }
  [[nodiscard]] const Comparison<FrameTrackStats>& GetComparisonFrameTrackStats() const {
    return comparison_frame_track_stats_;
  }

  [[nodiscard]] const CorrectedComparisonResult& GetComparisonResult(SFID sfid) const {
    return fid_to_corrected_comparison_results_.at(sfid);
  }

  [[nodiscard]] const absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols>&
  GetSfidToSymbols() const {
    return *sfid_to_symbols_;
  }

 private:
  Baseline<Counts> baseline_sampling_counts_;
  Baseline<FrameTrackStats> baseline_frame_track_stats_;

  Comparison<Counts> comparison_sampling_counts_;
  Comparison<FrameTrackStats> comparison_frame_track_stats_;

  absl::flat_hash_map<SFID, CorrectedComparisonResult> fid_to_corrected_comparison_results_;

  const absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols>* sfid_to_symbols_;
};

// The production code should rely on this alias, changes to RHS are not planned
using SamplingWithFrameTrackComparisonReport =
    SamplingWithFrameTrackComparisonReportTmpl<SamplingCounts, orbit_client_data::ScopeStats>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_