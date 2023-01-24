// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/VerticalSizer.h"

#include <QCursor>
#include <QGuiApplication>
#include <Qt>
#include <utility>

#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"

namespace orbit_gl {

VerticalSizer::VerticalSizer(CaptureViewElement* parent, const orbit_gl::Viewport* viewport,
                             const TimeGraphLayout* layout,
                             std::function<void(int /*x*/, int /*y*/)> on_drag_callback)
    : CaptureViewElement(parent, viewport, layout),
      on_drag_callback_(std::move(on_drag_callback)) {}

void VerticalSizer::SetHeight(float height) {
  if (height == height_) return;
  height_ = height;
  RequestUpdate(RequestUpdateScope::kDraw);
}

void VerticalSizer::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                           const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);
  Quad box = MakeBox(GetPos(), GetSize());
  primitive_assembler.AddBox(box, GlCanvas::kZValueMargin, GlCanvas::kBackgroundColor,
                             shared_from_this());
}

CaptureViewElement::EventResult VerticalSizer::OnMouseEnter() {
  EventResult event_result = CaptureViewElement::OnMouseEnter();
  if (QGuiApplication::instance() != nullptr) {
    QGuiApplication::setOverrideCursor(QCursor{Qt::SizeHorCursor});
  }
  return event_result;
}

CaptureViewElement::EventResult VerticalSizer::OnMouseLeave() {
  EventResult event_result = CaptureViewElement::OnMouseLeave();
  if (QGuiApplication::instance() != nullptr) {
    QGuiApplication::restoreOverrideCursor();
  }
  return event_result;
}

void VerticalSizer::OnDrag(int x, int y) { on_drag_callback_(x, y); }

std::unique_ptr<orbit_accessibility::AccessibleInterface>
VerticalSizer::CreateAccessibleInterface() {
  return nullptr;
}

}  // namespace orbit_gl