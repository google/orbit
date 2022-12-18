// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TriangleToggle.h"

#include <GteVector.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include "OrbitGl/AccessibleTriangleToggle.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"

using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

TriangleToggle::TriangleToggle(CaptureViewElement* parent, const orbit_gl::Viewport* viewport,
                               const TimeGraphLayout* layout, StateChangeHandler handler)
    : CaptureViewElement(parent, viewport, layout), handler_(std::move(handler)) {}

void TriangleToggle::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                            const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);

  const float z = GlCanvas::kZValueTrackText;

  const bool picking = draw_context.picking_mode != PickingMode::kNone;
  const Color white(255, 255, 255, 255);
  const Color grey(100, 100, 100, 255);
  Color color = is_collapsible_ ? white : grey;

  if (!picking) {
    const float half_width = GetWidth() / 2.f;
    const float half_height = GetHeight() / 2.f;
    const Vec2 midpoint = GetPos() + Vec2(half_width, half_height);

    // Down arrow vertices.
    std::array<Vec2, 6> v = {Vec2{0, 0}, {-1, -1}, {-1, 0}, {0, 1}, {1, 0}, {1, -1}};
    
    // Scale and translate vertices, flipping the y direction based on collapse state.
    Vec2 scale{half_width, is_collapsed_ ? half_height : -half_height};
    for (Vec2& vertex : v) {
      vertex = vertex*scale + midpoint;
    }

    // Arrow triangles.
    std::array<Triangle, 4> triangles = {
        Triangle{v[0], v[1], v[2]}, {v[0], v[2], v[3]}, {v[0], v[3], v[4]}, {v[0], v[4], v[5]}};
    
    for (const auto& triangle : triangles) {
      primitive_assembler.AddTriangle(triangle, z, color, shared_from_this());
    }
  } else {
    // When picking, draw a big square for easier picking.
    Quad box = MakeBox(GetPos(), Vec2(GetWidth(), GetHeight()));
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
