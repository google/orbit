// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACEPOINT_TRACK_H_
#define ORBIT_GL_TRACEPOINT_TRACK_H_

#include "Track.h"

class TracepointTrack : public Track {
 public:
  explicit TracepointTrack(TimeGraph* time_graph);

  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) override;

  void SetPos(float x, float y);

  // TODO(msandru): Override Track::Draw, Track::OnPick, Track::OnRelease(), Track::GetBoxTooltip()

 protected:
  int32_t thread_id_;
  Vec2 position_;
  TextBox thread_name_;
  Vec2 size_;
  Color color_;
};

#endif  // ORBIT_GL_TRACEPOINT_TRACK_H_
