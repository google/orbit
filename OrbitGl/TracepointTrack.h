// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACEPOINT_TRACK_H_
#define ORBIT_GL_TRACEPOINT_TRACK_H_

#include "GlCanvas.h"
#include "TimeGraph.h"
#include "Track.h"

class TracepointTrack : public EventTrack {
 public:
  explicit TracepointTrack(TimeGraph* time_graph);

  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) override;

  void SetPos(float x, float y);

  // TODO(msandru): Override Track::Draw, Track::OnPick, Track::OnRelease(), Track::GetBoxTooltip()
};

#endif  // ORBIT_GL_TRACEPOINT_TRACK_H_
