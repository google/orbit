// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_PAIRED_DATA_H_
#define MIZAR_DATA_MIZAR_PAIRED_DATA_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <utility>
#include <variant>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/ScopeId.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/FrameTrackManager.h"
#include "MizarData/MizarDataProvider.h"

namespace orbit_mizar_data {

// This class represents the data loaded from a capture that has been made aware of its counterpart
// it will be compared against. In particular, it is aware of the functions that has been sampled in
// the other capture. Also, it is aware of the sampled function ids assigned to the functions.
template <typename Data, typename FrameTracks, typename FrameTrackStats>
class MizarPairedDataTmpl {
  using SFID = ::orbit_mizar_base::SampledFunctionId;
  using TID = ::orbit_mizar_base::TID;
  using ScopeId = ::orbit_client_data::ScopeId;
  using RelativeTimeNs = ::orbit_mizar_base::RelativeTimeNs;
  using TimestampNs = ::orbit_mizar_base::TimestampNs;
  using AbsoluteAddress = ::orbit_mizar_base::AbsoluteAddress;

 public:
  struct WallClockAndActiveInvocationTimeStats {
    FrameTrackStats wall_clock_time;
    FrameTrackStats active_invocation_time;
  };

  MizarPairedDataTmpl(std::unique_ptr<Data> data,
                      absl::flat_hash_map<AbsoluteAddress, SFID> address_to_sfid)
      : data_(std::move(data)),
        address_to_sfid_(std::move(address_to_sfid)),
        frame_tracks_(data_.get()) {
    SetThreadNamesAndCallstackCounts();
  }

  // The function estimates how much of CPU-time has been actually spent by the threads in `tids`
  // during each of the frames. scope-id is used as a frame-track. This time does not include the
  // time the process was waiting, de-scheduled, or the VM itself was de-scheduled. The estimate is
  // obtained via counting how many callstack samples have been obtained during each frame and then
  // multiplying the counter by the sampling period.
  [[nodiscard]] std::vector<RelativeTimeNs> ActiveInvocationTimes(
      const absl::flat_hash_set<TID>& tids, FrameTrackId frame_track_id,
      RelativeTimeNs min_relative_time, RelativeTimeNs max_relative_time) const {
    return ReduceOverFrames<std::vector<RelativeTimeNs>>(
        frame_track_id, min_relative_time, max_relative_time,
        [this, &tids](std::vector<RelativeTimeNs>& times, Frame frame) {
          times.push_back(FrameActiveInvocationTime(tids, frame));
        });
  }

  [[nodiscard]] FrameTrackStats ActiveInvocationTimeStats(const absl::flat_hash_set<TID>& tids,
                                                          FrameTrackId frame_track_id,
                                                          RelativeTimeNs min_relative_time,
                                                          RelativeTimeNs max_relative_time) const {
    return ReduceOverFrames<FrameTrackStats>(
        frame_track_id, min_relative_time, max_relative_time,
        [this, &tids](FrameTrackStats& stats, Frame frame) {
          stats.UpdateStats(*FrameActiveInvocationTime(tids, frame));
        });
  }

  [[nodiscard]] WallClockAndActiveInvocationTimeStats WallClockAndActiveInvocationTimeStats(
      const absl::flat_hash_set<TID>& tids, FrameTrackId frame_track_id,
      RelativeTimeNs min_relative_time, RelativeTimeNs max_relative_time) const {
    return ReduceOverFrames<struct WallClockAndActiveInvocationTimeStats>(
        frame_track_id, min_relative_time, max_relative_time,
        [this, &tids](struct WallClockAndActiveInvocationTimeStats& stats, Frame frame) {
          stats.active_invocation_time.UpdateStats(*FrameActiveInvocationTime(tids, frame));
          stats.wall_clock_time.UpdateStats(*Sub(frame.end, frame.start));
        });
  }

  [[nodiscard]] const absl::flat_hash_map<TID, std::string>& TidToNames() const {
    return tid_to_names_;
  }

  [[nodiscard]] const absl::flat_hash_map<TID, uint64_t>& TidToCallstackSampleCounts() const {
    return tid_to_callstack_samples_counts_;
  }

  [[nodiscard]] absl::flat_hash_map<FrameTrackId, FrameTrackInfo> GetFrameTracks() const {
    return frame_tracks_.GetFrameTracks();
  }

  [[nodiscard]] std::vector<TimestampNs> GetFrameStarts(FrameTrackId id, TimestampNs min_start,
                                                        TimestampNs max_start) const {
    return frame_tracks_.GetFrameStarts(id, min_start, max_start);
  }

  // Action is a void callable that takes a single argument of type
  // `const std::vector<ScopeId>` representing a callstack sample.
  // `min_relative_timestamp_ns` and `max_relative_timestamp_ns` are the nanoseconds elapsed since
  // capture start.
  template <typename Action>
  void ForEachCallstackEvent(TID tid, RelativeTimeNs min_relative_timestamp,
                             RelativeTimeNs max_relative_timestamp, Action&& action) const {
    auto action_on_callstack_events =
        [this, &action](const orbit_client_data::CallstackEvent& event) -> void {
      const orbit_client_data::CallstackInfo* callstack =
          GetCallstackData().GetCallstack(event.callstack_id());
      const std::vector<SFID> sfids = CallstackWithSFIDs(callstack);
      std::invoke(action, sfids);
    };

    const auto [min_timestamp_ns, max_timestamp_ns] =
        RelativeToAbsoluteTimestampRange(min_relative_timestamp, max_relative_timestamp);
    ForEachCallstackEventOfTidInTimeRange(tid, min_timestamp_ns, max_timestamp_ns,
                                          action_on_callstack_events);
  }

  [[nodiscard]] RelativeTimeNs CaptureDurationNs() const {
    return Sub(orbit_mizar_base::TimestampNs(GetCallstackData().max_time()),
               data_->GetCaptureStartTimestampNs());
  }

 private:
  struct Frame {
    TimestampNs start;
    TimestampNs end;
  };

  // Op is a `void` callable that takes `Accumulator&` and `Frame`.
  template <typename Accumulator, typename Op>
  [[nodiscard]] Accumulator ReduceOverFrames(FrameTrackId frame_track_id,
                                             RelativeTimeNs min_relative_time,
                                             RelativeTimeNs max_relative_time, Op&& op) const {
    Accumulator accumulator;
    const auto [min_timestamp_ns, max_timestamp_ns] =
        RelativeToAbsoluteTimestampRange(min_relative_time, max_relative_time);
    const std::vector<TimestampNs> frame_starts =
        GetFrameStarts(frame_track_id, min_timestamp_ns, max_timestamp_ns);
    if (frame_starts.size() < 2) return accumulator;

    for (size_t i = 0; i + 1 < frame_starts.size(); ++i) {
      std::invoke(op, accumulator, Frame{frame_starts[i], frame_starts[i + 1]});
    }
    return accumulator;
  }

  [[nodiscard]] RelativeTimeNs FrameActiveInvocationTime(const absl::flat_hash_set<TID>& tids,
                                                         Frame frame) const {
    const uint64_t callstack_count = std::transform_reduce(
        std::begin(tids), std::end(tids), uint64_t{0}, std::plus<>(),
        [&](const TID tid) { return CountCallstackSamples(tid, frame.start, frame.end); });
    const RelativeTimeNs sampling_period = this->data_->GetNominalSamplingPeriodNs();
    return Times(sampling_period, callstack_count);
  }

  void SetThreadNamesAndCallstackCounts() {
    const absl::flat_hash_map<uint32_t, std::string>& thread_names =
        GetCaptureData().thread_names();
    GetCallstackData().ForEachCallstackEventInTimeRange(
        0, std::numeric_limits<uint64_t>::max(),
        [this, &thread_names](const orbit_client_data::CallstackEvent& event) {
          const TID tid{event.thread_id()};
          tid_to_callstack_samples_counts_[tid]++;

          if (tid_to_names_.contains(tid)) return;

          const auto tid_to_name = thread_names.find(*tid);
          std::string thread_name = (tid_to_name != thread_names.end()) ? tid_to_name->second : "";
          tid_to_names_.try_emplace(tid, std::move(thread_name));
        });
  }

  template <typename Action>
  void ForEachCallstackEventOfTidInTimeRange(TID tid, TimestampNs min_timestamp_ns,
                                             TimestampNs max_timestamp_ns,
                                             Action&& action_on_callstack_events) const {
    GetCallstackData().ForEachCallstackEventOfTidInTimeRange(
        *tid, *min_timestamp_ns, *max_timestamp_ns, action_on_callstack_events);
  }

  [[nodiscard]] uint64_t CountCallstackSamples(TID tid, TimestampNs min_timestamp_ns,
                                               TimestampNs max_timestamp_ns) const {
    uint64_t count = 0;
    ForEachCallstackEventOfTidInTimeRange(tid, min_timestamp_ns, max_timestamp_ns,
                                          [&count](const auto& /*callstack*/) { count++; });
    return count;
  }

  [[nodiscard]] TimestampNs ToAbsoluteTimestamp(RelativeTimeNs relative_time) const {
    return Add(data_->GetCaptureStartTimestampNs(), relative_time);
  }

  [[nodiscard]] std::pair<TimestampNs, TimestampNs> RelativeToAbsoluteTimestampRange(
      RelativeTimeNs min_relative_time, RelativeTimeNs max_relative_time) const {
    return std::make_pair(ToAbsoluteTimestamp(min_relative_time),
                          ToAbsoluteTimestamp(max_relative_time));
  }

  [[nodiscard]] std::vector<SFID> CallstackWithSFIDs(
      const orbit_client_data::CallstackInfo* callstack) const {
    if (callstack->frames().empty()) return {};
    if (callstack->type() != orbit_client_data::CallstackType::kComplete) {
      return CallstackWithSFIDs({callstack->frames()[0]});
    }
    return CallstackWithSFIDs(callstack->frames());
  }

  [[nodiscard]] std::vector<SFID> CallstackWithSFIDs(absl::Span<const uint64_t> frames) const {
    std::vector<SFID> result;
    orbit_mizar_base::ForEachFrame(frames, [this, &result](AbsoluteAddress address) {
      if (auto it = address_to_sfid_.find(address); it != address_to_sfid_.end()) {
        result.push_back(it->second);
      }
    });
    return result;
  }

  [[nodiscard]] const auto& GetCaptureData() const { return data_->GetCaptureData(); }

  [[nodiscard]] const orbit_client_data::CallstackData& GetCallstackData() const {
    return GetCaptureData().GetCallstackData();
  }

  std::unique_ptr<Data> data_;
  absl::flat_hash_map<AbsoluteAddress, SFID> address_to_sfid_;
  FrameTracks frame_tracks_;
  absl::flat_hash_map<TID, std::string> tid_to_names_;
  absl::flat_hash_map<TID, uint64_t> tid_to_callstack_samples_counts_;
};

using MizarPairedData =
    MizarPairedDataTmpl<MizarDataProvider, FrameTrackManager, orbit_client_data::ScopeStats>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_PAIRED_DATA_H_
