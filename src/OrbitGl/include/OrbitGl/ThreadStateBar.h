// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_STATE_BAR_H_
#define ORBIT_GL_THREAD_STATE_BAR_H_

#include <GteVector.h>
#include <stdint.h>

#include <memory>
#include <optional>
#include <string>

#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/ThreadBar.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

// This is a track dedicated to displaying thread states in different colors
// and with the corresponding tooltips.
// It is a thin sub-track of ThreadTrack, added above the callstack track (EventTrack).
// The colors are determined only by the states, not by the color assigned to the thread.
class ThreadStateBar final : public ThreadBar {
 public:
  explicit ThreadStateBar(CaptureViewElement* parent, OrbitApp* app,
                          const orbit_gl::TimelineInfoInterface* timeline_info,
                          orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                          const orbit_client_data::ModuleManager* module_manager,
                          const orbit_client_data::CaptureData* capture_data,
                          orbit_client_data::ThreadID thread_id);

  [[nodiscard]] float GetHeight() const override { return layout_->GetThreadStateTrackHeight(); }

  void OnPick(int x, int y) override;

  [[nodiscard]] bool IsEmpty() const override;

  [[nodiscard]] EventResult OnMouseMove(const Vec2& mouse_pos) override;
  [[nodiscard]] EventResult OnMouseLeave() override;

  void DrawThreadStateSliceOutline(PrimitiveAssembler& primitive_assembler,
                                   const orbit_client_data::ThreadStateSliceInfo& slice,
                                   const Color& outline_color) const;

 protected:
  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  void DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                          uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) override;
  [[nodiscard]] std::optional<orbit_client_data::ThreadStateSliceInfo> FindSliceFromWorldCoords(
      const Vec2& pos) const;

 private:
  std::string GetThreadStateSliceTooltip(PrimitiveAssembler& primitive_assembler,
                                         PickingId id) const;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_THREAD_STATE_TRACK_H_
