// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_
#define MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <utility>

#include "ClientData/ScopeStats.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/SampledFunctionId.h"
#include "OrbitBase/Logging.h"

namespace orbit_mizar_data {

// Whatever is usually referred to as "Statistical Test" we call "Comparison" in the project to save
// confusion with Unit tests.
struct ComparisonResult {
  double statistic{};

  // The term from Statistics. TL;DR: The smaller it is, the less we believe in the assumption under
  // test (e.g. no difference in active function time).
  double pvalue{};
};

struct CorrectedComparisonResult : public ComparisonResult {
  // result of multiplicity correction (a term from Statistics) for the particular comparison.
  double corrected_pvalue{};
};

// The struct represents the part of configuration relevant to one of the two captures under
// comparison.
struct HalfOfSamplingWithFrameTrackReportConfig {
  explicit HalfOfSamplingWithFrameTrackReportConfig(absl::flat_hash_set<uint32_t> tids,
                                                    uint64_t start_ns, uint64_t duration_ns,
                                                    uint64_t frame_track_scope_id)
      : tids(std::move(tids)),
        start_relative_ns(start_ns),
        duration_ns(duration_ns),
        frame_track_scope_id(frame_track_scope_id) {}

  absl::flat_hash_set<uint32_t> tids{};
  uint64_t start_relative_ns{};  // nanoseconds elapsed since capture start
  uint64_t duration_ns{};
  uint64_t frame_track_scope_id{};
};

struct InclusiveAndExclusive {
  uint64_t inclusive{};
  uint64_t exclusive{};
};

class SamplingCounts {
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
class SamplingWithFrameTrackComparisonReport {
 public:
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;

  explicit SamplingWithFrameTrackComparisonReport(
      Baseline<SamplingCounts> baseline_sampling_counts,
      Baseline<orbit_client_data::ScopeStats> baseline_frame_track_stats,
      Comparison<SamplingCounts> comparison_sampling_counts,
      Comparison<orbit_client_data::ScopeStats> comparison_frame_track_stats,
      absl::flat_hash_map<SFID, CorrectedComparisonResult> fid_to_corrected_comparison_results)
      : baseline_sampling_counts_(std::move(baseline_sampling_counts)),
        baseline_frame_track_stats_(std::move(baseline_frame_track_stats)),
        comparison_sampling_counts_(std::move(comparison_sampling_counts)),
        comparison_frame_track_stats_(std::move(comparison_frame_track_stats)),
        fid_to_corrected_comparison_results_(std::move(fid_to_corrected_comparison_results)) {}

  // TODO(b/236714217) de-template the getter
  template <template <typename> typename Wrapper>
  const Wrapper<SamplingCounts>& GetSamplingCounts() const {
    if constexpr (std::is_same_v<Wrapper<SamplingCounts>, Baseline<SamplingCounts>>) {
      return baseline_sampling_counts_;
    } else {
      return comparison_sampling_counts_;
    }
  }

  // TODO(b/236714217) de-template the getter
  template <template <typename> typename Wrapper>
  const Wrapper<orbit_client_data::ScopeStats>& GetFrameTrackStats() const {
    if constexpr (std::is_same_v<Wrapper<orbit_client_data::ScopeStats>,
                                 Baseline<orbit_client_data::ScopeStats>>) {
      return baseline_frame_track_stats_;
    } else {
      return comparison_frame_track_stats_;
    }
  }

  [[nodiscard]] const CorrectedComparisonResult& GetComparisonResult(SFID sfid) const {
    return fid_to_corrected_comparison_results_.at(sfid);
  }

 private:
  Baseline<SamplingCounts> baseline_sampling_counts_;
  Baseline<orbit_client_data::ScopeStats> baseline_frame_track_stats_;

  Comparison<SamplingCounts> comparison_sampling_counts_;
  Comparison<orbit_client_data::ScopeStats> comparison_frame_track_stats_;

  absl::flat_hash_map<SFID, CorrectedComparisonResult> fid_to_corrected_comparison_results_;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_