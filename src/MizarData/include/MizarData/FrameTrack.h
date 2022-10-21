// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_FRAME_TRACK_H_
#define MIZAR_DATA_MIZAR_FRAME_TRACK_H_

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_data {

struct FrameTrackIdTag {};
// The type is used identify a frame track in Mizar. It can be either ETW or a scope.
using FrameTrackId =
    orbit_base::Typedef<FrameTrackIdTag, std::variant<orbit_client_data::ScopeId,
                                                      orbit_grpc_protos::PresentEvent::Source>>;

struct FrameTrackInfoTag {};
// The class describes a frame track
using FrameTrackInfo =
    orbit_base::Typedef<FrameTrackInfoTag, std::variant<orbit_client_data::ScopeInfo,
                                                        orbit_grpc_protos::PresentEvent::Source>>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_FRAME_TRACK_H_