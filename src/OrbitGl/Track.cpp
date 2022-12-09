// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/Track.h"

#include <GteVector.h>

#include "ClientData/CaptureData.h"
#include "OrbitGl/AccessibleTrack.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TrackRenderHelper.h"
#include "OrbitGl/Viewport.h"

using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

Track::Track(CaptureViewElement* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
             orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
             const orbit_client_data::ModuleManager* module_manager,
             const orbit_client_data::CaptureData* capture_data)
    : CaptureViewElement(parent, viewport, layout),
      pinned_{false},
      timeline_info_(timeline_info),
      module_manager_(module_manager),
      capture_data_(capture_data) {
  header_ = std::make_shared<orbit_gl::TrackHeader>(this, viewport, layout, this);
}

void Track::OnPick(int x, int y) {
  CaptureViewElement::OnPick(x, y);
  SelectTrack();
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> Track::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleTrack>(this, layout_);
}

void Track::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                   const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);

  // An indentation level > 0 means this track is nested under a parent track. In this case, we
  // don't want to draw any background for this track.
  const bool draw_background = GetIndentationLevel() == 0 && layout_->GetDrawTrackBackground();
  const bool picking = draw_context.picking_mode != PickingMode::kNone;

  // Tab and content are handled by child elements or inheriting classes, so we can early-out here
  // if we don't need to draw any tab background.
  // Same for picking - the track background is not pickable
  if (picking || !draw_background) return;

  const float x0 = GetPos()[0];
  const float y0 = GetPos()[1];
  const float header_height = header_->GetHeight();
  const float track_z = GlCanvas::kZValueTrack;

  Color track_background_color = GetTrackBackgroundColor();

  Vec2 track_size = GetSize();

  // Draw rounded corners.
  float radius = std::min(layout_->GetRoundingRadius(), layout_->GetTrackTabHeight() * 0.5f);
  auto sides = static_cast<uint32_t>(layout_->GetRoundingNumSides() + 0.5f);
  auto rounded_corner = orbit_gl::GetRoundedCornerMask(radius, sides);

  // This draws the non-tab content of the track. The tab itself is a separate CaptureViewElement
  // child of the track.
  // Note that the top-left corner is not rounded due to the design of the track tab sitting
  // in the top left.
  //
  // [ Header ] ________________________  content_top_right
  // |                                  `
  // |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|
  // \__________________________________/
  // content_bottom_left                 content_bottom_right
  //
  Vec2 content_bottom_left(x0, y0 + track_size[1]);
  Vec2 content_bottom_right(x0 + track_size[0], y0 + track_size[1]);

  // The track content starts underneath the track header, and we only draw the background
  // starting
  // there. This could be prevented by moving the track content into a separate child, but in the
  // end, the design of track and track header will need to influence each other...
  Vec2 content_top_right(x0 + track_size[0], y0 + header_height);
  Vec2 content_top_left(x0, y0 + header_height);

  auto shared_this = shared_from_this();
  orbit_gl::DrawTriangleFan(primitive_assembler, rounded_corner, content_bottom_left,
                            GlCanvas::kBackgroundColor, 0, track_z, shared_this);
  orbit_gl::DrawTriangleFan(primitive_assembler, rounded_corner, content_bottom_right,
                            GlCanvas::kBackgroundColor, -90.f, track_z, shared_this);
  orbit_gl::DrawTriangleFan(primitive_assembler, rounded_corner, content_top_right,
                            GlCanvas::kBackgroundColor, 180.f, track_z, shared_this);

  // Draw track's content background.
  Quad box = MakeBox(content_top_left, Vec2(GetWidth(), GetHeight() - header_height));
  primitive_assembler.AddBox(box, track_z, track_background_color, shared_from_this());
}

void Track::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  header_->SetWidth(layout_->GetTrackTabWidth());
  header_->SetPos(GetPos()[0] + layout_->GetTrackTabOffset(), GetPos()[1]);
  UpdatePositionOfSubtracks();
}

void Track::SetPinned(bool value) { pinned_ = value; }

void Track::DragBy(float delta_y) {
  if (!Draggable()) return;

  SetPos(GetPos()[0], GetPos()[1] + delta_y);
}

Color Track::GetTrackBackgroundColor() const {
  uint32_t capture_process_id = capture_data_->process_id();

  if (GetProcessId() != orbit_base::kInvalidProcessId && GetProcessId() != capture_process_id &&
      GetType() != Type::kSchedulerTrack) {
    const Color external_process_color(30, 30, 40, 255);
    return external_process_color;
  }

  const Color dark_grey(50, 50, 50, 255);
  return dark_grey;
}

bool Track::ShouldBeRendered() const {
  return CaptureViewElement::ShouldBeRendered() && !IsEmpty();
}

float Track::DetermineZOffset() const {
  float result = 0.f;
  if (IsPinned()) {
    result = GlCanvas::kZOffsetPinnedTrack;
  } else if (IsMoving()) {
    result = GlCanvas::kZOffsetMovingTrack;
  }
  return result;
}

void Track::SetCollapsed(bool collapsed) {
  header_->GetCollapseToggle()->SetCollapsed(collapsed);

  RequestUpdate();
}

void Track::SetHeadless(bool value) {
  if (headless_ == value) return;

  headless_ = value;
  header_->SetVisible(!value);
  RequestUpdate();
}

void Track::SetIndentationLevel(uint32_t level) {
  if (level == indentation_level_) return;

  indentation_level_ = level;
  RequestUpdate();
}
