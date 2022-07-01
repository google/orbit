// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TriangleToggle.h"

#include <GteVector.h>
#include <math.h>

#include <utility>

#include "AccessibleTriangleToggle.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "PrimitiveAssembler.h"
#include "Track.h"

using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

TriangleToggle::TriangleToggle(CaptureViewElement* parent, const orbit_gl::Viewport* viewport,
                               const TimeGraphLayout* layout, StateChangeHandler handler)
    : CaptureViewElement(parent, viewport, layout), handler_(std::move(handler)) {}

void TriangleToggle::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                            const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);

  const float z = GlCanvas::kZValueTrack;

  const bool picking = draw_context.picking_mode != PickingMode::kNone;
  const Color kWhite(255, 255, 255, 255);
  const Color kGrey(100, 100, 100, 255);
  Color color = is_collapsible_ ? kWhite : kGrey;

  // Draw triangle.
  const Vec2 midpoint = GetPos() + Vec2(GetWidth() / 2, GetHeight() / 2);
  const float triangle_side_length = std::min(GetWidth(), GetHeight());
  const float kCos30 = sqrtf(3.f) / 2;
  const float triangle_height = triangle_side_length * kCos30;

  const float half_triangle_height = ceilf(triangle_height / 2);
  const float half_triangle_side_length = triangle_side_length / 2;

  if (!picking) {
    Triangle triangle;
    if (is_collapsed_) {
      triangle = Triangle(midpoint + Vec2(-half_triangle_height, half_triangle_side_length),
                          midpoint + Vec2(-half_triangle_height, -half_triangle_side_length),
                          midpoint + Vec2(half_triangle_height, 0.f));
    } else {
      triangle = Triangle(midpoint + Vec2(half_triangle_side_length, -half_triangle_height),
                          midpoint + Vec2(-half_triangle_side_length, -half_triangle_height),
                          midpoint + Vec2(0.f, half_triangle_height));
    }
    primitive_assembler.AddTriangle(triangle, z, color, shared_from_this());
  } else {
    // When picking, draw a big square for easier picking.
    Quad box = MakeBox(midpoint - Vec2(half_triangle_side_length, half_triangle_side_length),
                       Vec2(triangle_side_length, triangle_side_length));
    primitive_assembler.AddBox(box, z, color, shared_from_this());
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
