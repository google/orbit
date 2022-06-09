// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_
#define MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <utility>

#include "MizarData/SampledFunctionId.h"
#include "OrbitBase/Logging.h"

namespace orbit_mizar_data {

// The struct represents the part of configuration relevant to one of the two captures under
// comparison.
struct HalfOfSamplingWithFrameTrackReportConfig {
  explicit HalfOfSamplingWithFrameTrackReportConfig(absl::flat_hash_set<uint32_t> tids,
                                                    uint64_t start_ns, uint64_t duration_ns)
      : tids(std::move(tids)), start_ns(start_ns), duration_ns(duration_ns) {}

  absl::flat_hash_set<uint32_t> tids{};
  uint64_t start_ns{};
  uint64_t duration_ns{};
};

struct BaselineSamplingWithFrameTrackReportConfig : HalfOfSamplingWithFrameTrackReportConfig {
  explicit BaselineSamplingWithFrameTrackReportConfig(absl::flat_hash_set<uint32_t> tids,
                                                      uint64_t start_ns, uint64_t duration_ns)
      : HalfOfSamplingWithFrameTrackReportConfig(std::move(tids), start_ns, duration_ns) {}
};

struct ComparisonSamplingWithFrameTrackReportConfig : HalfOfSamplingWithFrameTrackReportConfig {
  explicit ComparisonSamplingWithFrameTrackReportConfig(absl::flat_hash_set<uint32_t> tids,
                                                        uint64_t start_ns, uint64_t duration_ns)
      : HalfOfSamplingWithFrameTrackReportConfig(std::move(tids), start_ns, duration_ns) {}
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

  [[nodiscard]] uint64_t GetInclusiveCnt(SFID sfid) const {
    if (const auto it = counts_.find(sfid); it != counts_.end()) return it->second.inclusive;
    return 0;
  }
  [[nodiscard]] uint64_t GetExclusiveCnt(SFID sfid) const {
    if (const auto it = counts_.find(sfid); it != counts_.end()) return it->second.exclusive;
    return 0;
  }

  [[nodiscard]] double GetInclusiveRate(SFID sfid) const {
    if (total_callstacks_ == 0) return 0;
    return static_cast<double>(GetInclusiveCnt(sfid)) / GetTotalCallstacks();
  }
  [[nodiscard]] double GetExclusiveRate(SFID sfid) const {
    if (total_callstacks_ == 0) return 0;
    return static_cast<double>(GetExclusiveCnt(sfid)) / GetTotalCallstacks();
  }

  [[nodiscard]] uint64_t GetTotalCallstacks() const { return total_callstacks_; }

 private:
  absl::flat_hash_map<SFID, InclusiveAndExclusive> counts_;
  uint64_t total_callstacks_{};
};

struct SamplingWithFrameTrackComparisonReport {
  SamplingCounts baseline_sampling_counts;
  SamplingCounts comparison_sampling_counts;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_SAMPLING_WITH_FRAME_TRACK_COMPARISON_REPORT_H_