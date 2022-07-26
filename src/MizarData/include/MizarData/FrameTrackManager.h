// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_FRAME_TRACK_MANAGER_H_
#define MIZAR_DATA_MIZAR_FRAME_TRACK_MANAGER_H_

#include <absl/algorithm/container.h>
#include <absl/functional/bind_front.h>

#include "ClientData/CaptureData.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/MizarData.h"
#include "MizarData/MizarDataProvider.h"

namespace orbit_mizar_data {

template <typename Data>
class FrameTrackManagerTmpl {
  using ScopeId = ::orbit_client_data::ScopeId;
  using ScopeType = ::orbit_client_data::ScopeType;
  using PresentEvent = orbit_grpc_protos::PresentEvent;

 public:
  explicit FrameTrackManagerTmpl(const Data* data) : data_(data) {}

  [[nodiscard]] absl::flat_hash_map<FrameTrackId, FrameTrackInfo> GetFrameTracks() const {
    const std::vector<ScopeId> scope_ids = GetCaptureData().GetAllProvidedScopeIds();
    absl::flat_hash_map<FrameTrackId, FrameTrackInfo> result;
    for (const ScopeId scope_id : scope_ids) {
      const orbit_client_data::ScopeInfo scope_info = GetCaptureData().GetScopeInfo(scope_id);
      if (scope_info.GetType() == ScopeType::kDynamicallyInstrumentedFunction ||
          scope_info.GetType() == ScopeType::kApiScope) {
        result.try_emplace(FrameTrackId(scope_id), scope_info);
      }
    }

    for (auto& [source, unused_events] : data_->source_to_present_events()) {
      result.try_emplace(FrameTrackId(source), source);
    }
    return result;
  }

  [[nodiscard]] std::vector<FrameStartNs> GetFrameStarts(FrameTrackId id, FrameStartNs min_start,
                                                         FrameStartNs max_start) const {
    return Visit(
        absl::bind_front(&FrameTrackManagerTmpl::GetScopeFrameStarts, this, min_start, max_start),
        absl::bind_front(&FrameTrackManagerTmpl::GetEtwFrameStarts, this, min_start, max_start),
        id);
  }

 private:
  [[nodiscard]] std::vector<FrameStartNs> GetScopeFrameStarts(FrameStartNs min_start,
                                                              FrameStartNs max_start,
                                                              ScopeId scope_id) const {
    const std::vector<const TimerInfo*> timers =
        GetCaptureData().GetTimersForScope(scope_id, *min_start, *max_start);

    std::vector<FrameStartNs> result;
    result.reserve(timers.size());
    absl::c_transform(timers, std::back_inserter(result),
                      [](const TimerInfo* timer) { return FrameStartNs(timer->start()); });
    return result;
  }

  [[nodiscard]] std::vector<FrameStartNs> GetEtwFrameStarts(FrameStartNs min_start,
                                                            FrameStartNs max_start,
                                                            PresentEvent::Source etw_source) const {
    const auto& source_to_present_events = data_->source_to_present_events();

    const auto it = source_to_present_events.find(etw_source);
    if (it == source_to_present_events.end()) {
      return {};
    }
    std::vector<FrameStartNs> result;
    const std::vector<PresentEvent>& events = it->second;
    for (const PresentEvent& event : events) {
      const FrameStartNs event_start(event.begin_timestamp_ns());
      if (min_start <= event_start && event_start <= max_start) {
        result.push_back(event_start);
      }
    }
    return result;
  }

  const auto& GetCaptureData() const { return data_->GetCaptureData(); }

  const Data* data_;
};

using FrameTrackManager = FrameTrackManagerTmpl<MizarDataProvider>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_FRAME_TRACK_MANAGER_H_
