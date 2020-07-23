// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TriangleToggle.h"

#include "GlCanvas.h"
#include "OpenGl.h"

TriangleToggle::TriangleToggle(State initial_state, StateChangeHandler handler,
                               TimeGraph* time_graph)
    : state_(initial_state),
      initial_state_(initial_state),
      handler_(handler),
      time_graph_(time_graph) {}

void TriangleToggle::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  Batcher* batcher = canvas->GetBatcher();

  const float picking = picking_mode != PickingMode::kNone;
  const Color kWhite(255, 255, 255, 255);
  const Color kGrey(100, 100, 100, 255);
  Color color = state_ == State::kInactive ? kGrey : kWhite;

  if (picking) {
    PickingManager& picking_manager = canvas->GetPickingManager();
    color = picking_manager.GetPickableColor(this, PickingID::BatcherId::UI);
  }

  // Draw triangle.
  static float half_sqrt_three = 0.5f * sqrtf(3.f);
  float half_w = 0.5f * size_;
  float half_h = half_sqrt_three * half_w;

  if (!picking) {
    Vec3 position(pos_[0], pos_[1], 0.0f);

    Triangle triangle;
    if (state_ == State::kCollapsed) {
      triangle = Triangle(position + Vec3(-half_h, half_w, 0.f),
                          position + Vec3(-half_h, -half_w, 0.f),
                          position + Vec3(half_w, 0.f, 0.f));
    } else {
      triangle = Triangle(position + Vec3(half_w, half_h, 0.f),
                          position + Vec3(-half_w, half_h, 0.f),
                          position + Vec3(0.f, -half_w, 0.f));
    }
    batcher->AddTriangle(triangle, color, PickingID::PICKABLE);
  } else {
    // When picking, draw a big square for easier picking.
    float original_width = 2 * half_w;
    float large_width = 2 * original_width;
    Box box(Vec2(pos_[0] - original_width, pos_[1] - original_width),
            Vec2(large_width, large_width), 0.f);
    batcher->AddBox(box, color, PickingID::PICKABLE);
  }
}

void TriangleToggle::OnPick(int /*x*/, int /*y*/) {}

void TriangleToggle::OnRelease() {
  if (IsInactive()) {
    return;
  }

  state_ = IsCollapsed() ? State::kExpanded : State::kCollapsed;
  handler_(state_);
  time_graph_->NeedsUpdate();
}

void TriangleToggle::SetState(State state) { state_ = state; }
