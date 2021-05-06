// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_STATE_BAR_H_
#define ORBIT_GL_THREAD_STATE_BAR_H_

#include <GteVector.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "CaptureViewElement.h"
#include "CoreMath.h"
#include "ThreadBar.h"
#include "Viewport.h"

namespace orbit_gl {

// This is a track dedicated to displaying thread states in different colors
// and with the corresponding tooltips.
// It is a thin sub-track of ThreadTrack, added above the callstack track (EventTrack).
// The colors are determined only by the states, not by the color assigned to the thread.
class ThreadStateBar final : public ThreadBar {
 public:
  explicit ThreadStateBar(CaptureViewElement* parent, OrbitApp* app, TimeGraph* time_graph,
                          orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                          const orbit_client_model::CaptureData* capture_data,
                          orbit_client_data::ThreadID thread_id);

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset) override;

  void OnPick(int x, int y) override;

  [[nodiscard]] bool IsEmpty() const override;

 private:
  std::string GetThreadStateSliceTooltip(Batcher* batcher, PickingId id) const;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_THREAD_STATE_TRACK_H_
