// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_FRAME_TRACK_H_
#define MIZAR_DATA_MIZAR_FRAME_TRACK_H_

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Typedef.h"
#include "stdint.h"

namespace orbit_mizar_data {

struct FrameTrackIdTag {};
// The type is used identify a frame track in Mizar. It can be either ETW or a scope.
using FrameTrackId =
    orbit_base::Typedef<FrameTrackIdTag, std::variant<orbit_client_data::ScopeId,
                                                      orbit_grpc_protos::PresentEvent::Source>>;

struct FrameStartNsTag {};
// Absolute timestamp of the capture start in nanos
using FrameStartNs = orbit_base::Typedef<FrameStartNsTag, uint64_t>;

struct FrameTrackInfoTag {};
// The class describes a frame track
using FrameTrackInfo =
    orbit_base::Typedef<FrameTrackInfoTag, std::variant<orbit_client_data::ScopeInfo,
                                                        orbit_grpc_protos::PresentEvent::Source>>;

// The function allows for more convenient calls to `std::visit`.
template <typename Tag, typename First, typename Second, typename ActionOnFirst,
          typename ActionOnSecond>
auto visit(ActionOnFirst&& action_on_scope_info, ActionOnSecond&& action_on_etw_source,
           const orbit_base::Typedef<Tag, std::variant<First, Second>>& host) {
  return std::visit(
      [&action_on_scope_info, &action_on_etw_source](const auto& info) {
        using InfoT = std::decay_t<decltype(info)>;
        if constexpr (std::is_same_v<InfoT, First>) {
          return std::invoke(std::forward<ActionOnFirst>(action_on_scope_info), info);
        }
        if constexpr (std::is_same_v<InfoT, Second>) {
          return std::invoke(std::forward<ActionOnSecond>(action_on_etw_source), info);
        }
      },
      *host);
}

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_FRAME_TRACK_H_