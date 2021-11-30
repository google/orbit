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
#include "Track.h"

TriangleToggle::TriangleToggle(StateChangeHandler handler, orbit_gl::Viewport* viewport,
                               TimeGraphLayout* layout, Track* track)
    : CaptureViewElement(track, viewport, layout), handler_(std::move(handler)) {}

void TriangleToggle::DoDraw(Batcher& batcher, TextRenderer& text_renderer,
                            const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(batcher, text_renderer, draw_context);

  const float z = GlCanvas::kZValueTrack;

  const bool picking = draw_context.picking_mode != PickingMode::kNone;
  const Color kWhite(255, 255, 255, 255);
  const Color kGrey(100, 100, 100, 255);
  Color color = is_collapsible_ ? kWhite : kGrey;

  // Draw triangle.
  static float half_sqrt_three = 0.5f * sqrtf(3.f);
  float half_triangle_base_width = 0.5f * GetWidth();
  float half_triangle_height = half_sqrt_three * 0.5f * GetHeight();

  const Vec2 pos = GetPos();
  if (!picking) {
    Vec3 position(pos[0], pos[1], 0.0f);

    Triangle triangle;
    if (is_collapsed_) {
      triangle = Triangle(position + Vec3(-half_triangle_height, half_triangle_base_width, z),
                          position + Vec3(-half_triangle_height, -half_triangle_base_width, z),
                          position + Vec3(half_triangle_base_width, 0.f, z));
    } else {
      triangle = Triangle(position + Vec3(half_triangle_base_width, -half_triangle_height, z),
                          position + Vec3(-half_triangle_base_width, -half_triangle_height, z),
                          position + Vec3(0.f, half_triangle_base_width, z));
    }
    batcher.AddTriangle(triangle, color, shared_from_this());
  } else {
    // When picking, draw a big square for easier picking.
    float original_width = 2 * half_triangle_base_width;
    float large_width = 2 * original_width;
    Box box(Vec2(pos[0] - original_width, pos[1] - original_width), Vec2(large_width, large_width),
            z);
    batcher.AddBox(box, color, shared_from_this());
  }
}

void TriangleToggle::OnRelease() {
  // Do not change the internal state when the toggle is not collapsible.
  if (!IsCollapsible()) {
    return;
  }

  CaptureViewElement::OnRelease();
  is_collapsed_ = !is_collapsed_;

  handler_(is_collapsed_);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface>
TriangleToggle::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleTriangleToggle>(this);
}
