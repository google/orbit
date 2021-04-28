// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TriangleToggle.h"

#include <GteVector.h>
#include <math.h>

#include <utility>

#include "AccessibleTriangleToggle.h"
#include "Batcher.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "TimeGraph.h"
#include "Track.h"

TriangleToggle::TriangleToggle(State initial_state, StateChangeHandler handler,
                               TimeGraph* time_graph, orbit_gl::Viewport* viewport,
                               TimeGraphLayout* layout, Track* track)
    : CaptureViewElement(track, time_graph, viewport, layout),
      track_(track),
      state_(initial_state),
      initial_state_(initial_state),
      handler_(std::move(handler)) {
  SetSize(10.f, 10.f);
}

void TriangleToggle::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  CaptureViewElement::Draw(canvas, picking_mode, z_offset);

  Batcher* ui_batcher = canvas->GetBatcher();
  const float z = GlCanvas::kZValueTrack + z_offset;

  const bool picking = picking_mode != PickingMode::kNone;
  const Color kWhite(255, 255, 255, 255);
  const Color kGrey(100, 100, 100, 255);
  Color color = state_ == State::kInactive ? kGrey : kWhite;

  // Draw triangle.
  static float half_sqrt_three = 0.5f * sqrtf(3.f);
  float half_w = 0.5f * size_[0];
  float half_h = half_sqrt_three * half_w;

  if (!picking) {
    Vec3 position(pos_[0], pos_[1], 0.0f);

    Triangle triangle;
    if (state_ == State::kCollapsed) {
      triangle = Triangle(position + Vec3(-half_h, half_w, z), position + Vec3(-half_h, -half_w, z),
                          position + Vec3(half_w, 0.f, z));
    } else {
      triangle = Triangle(position + Vec3(half_w, half_h, z), position + Vec3(-half_w, half_h, z),
                          position + Vec3(0.f, -half_w, z));
    }
    ui_batcher->AddTriangle(triangle, color, shared_from_this());
  } else {
    // When picking, draw a big square for easier picking.
    float original_width = 2 * half_w;
    float large_width = 2 * original_width;
    Box box(Vec2(pos_[0] - original_width, pos_[1] - original_width),
            Vec2(large_width, large_width), z);
    ui_batcher->AddBox(box, color, shared_from_this());
  }
}

void TriangleToggle::OnRelease() {
  if (IsInactive()) {
    return;
  }

  CaptureViewElement::OnRelease();

  state_ = IsCollapsed() ? State::kExpanded : State::kCollapsed;
  handler_(state_);
}

void TriangleToggle::SetState(State state, InitialStateUpdate behavior) {
  state_ = state;
  if (behavior == InitialStateUpdate::kReplaceInitialState) {
    initial_state_ = state;
  }
}

std::unique_ptr<orbit_accessibility::AccessibleInterface>
TriangleToggle::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleTriangleToggle>(this, track_);
}
