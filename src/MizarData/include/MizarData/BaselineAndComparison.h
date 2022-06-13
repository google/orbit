// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_H_

#include <stdint.h>

#include <string>
#include <type_traits>

#include "ClientData/CallstackData.h"
#include "ClientData/CaptureData.h"
#include "MizarData/MizarData.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/NonWrappingAddition.h"
#include "MizarData/SampledFunctionId.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"

namespace orbit_mizar_data {

template <typename T, typename U>
using EnableIfUConvertibleToU = std::enable_if_t<std::is_convertible_v<U, T>>;

template <typename T>
class BaselineOrComparison {
 public:
  ~BaselineOrComparison() = default;

  const T* operator->() const { return &value_; }
  T* operator->() { return &value_; }

 protected:
  template <typename U, typename = EnableIfUConvertibleToU<U, T>>
  explicit BaselineOrComparison(U&& value) : value_(std::forward<T>(value)) {}
  BaselineOrComparison(BaselineOrComparison&& other) = default;

 private:
  T value_;
};

template <typename T>
class Baseline : public BaselineOrComparison<T> {
 public:
  template <typename U, typename = EnableIfUConvertibleToU<U, T>>
  explicit Baseline(U&& value) : BaselineOrComparison<T>(std::forward<U>(value)) {}
};

template <typename T>
class Comparison : public BaselineOrComparison<T> {
 public:
  template <typename U, typename = EnableIfUConvertibleToU<U, T>>
  explicit Comparison(U&& value) : BaselineOrComparison<T>(std::forward<U>(value)) {}
};

template <typename T, typename... Args>
Baseline<T> MakeBaseline(Args&&... args) {
  return Baseline<T>(T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
Comparison<T> MakeComparison(Args&&... args) {
  return Comparison<T>(T(std::forward<Args>(args)...));
}

// The class owns the data from two capture files via owning two instances of
// `PairedData`. Also owns the map from sampled function ids to the
// corresponding function names.
template <typename PairedData>
class BaselineAndComparisonTmpl {
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
    return {MakeCounts(baseline_, baseline_config), MakeCounts(comparison_, comparison_config),
            MakeFrameTrackStats(baseline_, baseline_config),
            MakeFrameTrackStats(comparison_, comparison_config)};
  }

 private:
  template <template <class> class Wrapper>
  [[nodiscard]] orbit_client_data::ScopeStats MakeFrameTrackStats(
      const Wrapper<PairedData>& data,
      const Wrapper<HalfOfSamplingWithFrameTrackReportConfig>& config) const {
    const std::vector<uint64_t> active_invocation_times = data->ActiveInvocationTimes(
        config->tids, config->frame_track_scope_id, config->start_relative_ns,
        NonWrappingAddition(config->start_relative_ns, config->duration_ns));
    orbit_client_data::ScopeStats stats;
    for (const uint64_t active_invocation_time : active_invocation_times) {
      stats.UpdateStats(active_invocation_time);
    }
    return stats;
  }

  template <template <class> class Wrapper>
  [[nodiscard]] SamplingCounts MakeCounts(
      const Wrapper<PairedData>& data,
      const Wrapper<HalfOfSamplingWithFrameTrackReportConfig>& config) const {
    uint64_t total_callstacks = 0;
    absl::flat_hash_map<SFID, InclusiveAndExclusive> counts;
    for (const uint32_t tid : config->tids) {
      data->ForEachCallstackEvent(
          tid, config->start_relative_ns,
          NonWrappingAddition(config->start_relative_ns, config->duration_ns),
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

  Baseline<PairedData> baseline_;
  Comparison<PairedData> comparison_;
  absl::flat_hash_map<SFID, std::string> sfid_to_name_;
};

using BaselineAndComparison = BaselineAndComparisonTmpl<MizarPairedData>;

BaselineAndComparison CreateBaselineAndComparison(std::unique_ptr<MizarDataProvider> baseline,
                                                  std::unique_ptr<MizarDataProvider> comparison);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_H_