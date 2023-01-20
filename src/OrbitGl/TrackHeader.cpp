// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TrackHeader.h"

#include <algorithm>

#include "OrbitBase/Logging.h"
#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

TrackHeader::TrackHeader(CaptureViewElement* parent, const Viewport* viewport,
                         const TimeGraphLayout* layout, TrackControlInterface* track)
    : CaptureViewElement(parent, viewport, layout), track_(track) {
  ORBIT_CHECK(track_ != nullptr);
  collapse_toggle_ = std::make_shared<TriangleToggle>(
      this, viewport, layout, [this](bool /*is_collapsed*/) { RequestUpdate(); });
}

void TrackHeader::OnPick(int x, int y) {
  CaptureViewElement::OnPick(x, y);

  track_->SelectTrack();
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> TrackHeader::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleCaptureViewElement>(
      this, track_->GetName() + "_tab", orbit_accessibility::AccessibilityRole::PageTab);
}

void TrackHeader::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                         const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);

  const bool picking = draw_context.picking_mode != PickingMode::kNone;

  const float x0 = GetPos()[0];
  const float y0 = GetPos()[1];
  const float track_z = GlCanvas::kZValueTrackHeader;
  const float text_z = GlCanvas::kZValueTrackText;

  // Draw tab background
  const float label_height = height_;
  const float label_width = GetWidth();

  const float indentation_x0 =
      x0 + (track_->GetIndentationLevel() * layout_->GetTrackIndentOffset());

  if (track_->GetIndentationLevel() == 0 && layout_->GetDrawTrackHeaderBackground()) {
    Quad box = MakeBox(Vec2(indentation_x0, y0), Vec2(label_width, label_height));
    primitive_assembler.AddBox(box, track_z, track_->GetTrackBackgroundColor(), shared_from_this());
  }

  // Early-out: In picking mode, don't draw the text.
  if (picking) {
    return;
  }

  // Draw label.

  uint32_t font_size = layout_->GetFontSize();
  // For the first 5 indentations, we decrease the font_size by 10 percent points (per
  // indentation).
  constexpr uint32_t kMaxIndentationLevel = 5;
  uint32_t capped_indentation_level = std::min(track_->GetIndentationLevel(), kMaxIndentationLevel);
  font_size = (font_size * (10 - capped_indentation_level)) / 10;
  float label_offset_x = layout_->GetTrackLabelOffsetX();

  const Color color =
      track_->IsTrackSelected() ? GlCanvas::kTabTextColorSelected : Color(255, 255, 255, 255);

  TextRenderer::TextFormatting formatting{font_size, color, label_width - label_offset_x};
  formatting.valign = TextRenderer::VAlign::Middle;

  text_renderer.AddTextTrailingCharsPrioritized(
      track_->GetLabel().c_str(), indentation_x0 + label_offset_x,
      y0 + layout_->GetTextBoxHeight() * 0.5f + GetVerticalLabelOffset(), text_z, formatting,
      track_->GetNumberOfPrioritizedTrailingCharacters());
}

void orbit_gl::TrackHeader::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  UpdateCollapseToggle();
}

void TrackHeader::UpdateCollapseToggle() {
  const float x0 = GetPos()[0] + layout_->GetTrackIndentOffset() * track_->GetIndentationLevel();
  const float size = layout_->GetCollapseButtonSize(track_->GetIndentationLevel());

  constexpr float kOffsetY = 1.f;
  const float toggle_x_pos = x0 + layout_->GetCollapseButtonOffset();
  const float toggle_y_pos =
      GetPos()[1] + layout_->GetTextBoxHeight() * 0.5f - size * 0.5f + kOffsetY;

  Vec2 toggle_pos = Vec2(toggle_x_pos, toggle_y_pos + GetVerticalLabelOffset());

  collapse_toggle_->SetWidth(size);
  collapse_toggle_->SetHeight(size);
  collapse_toggle_->SetPos(toggle_pos[0], toggle_pos[1]);

  // Update the "collapsible" property of the triangle toggle to match the same property in the
  // parent track. This makes sure that changes to the track "collapsible" property are correctly
  // disabling the triangle toggle, even if they change during runtime.
  collapse_toggle_->SetIsCollapsible(track_->IsCollapsible());
}

float TrackHeader::GetVerticalLabelOffset() const {
  // TODO: Track hierarchy refactor, remove hack below.
  if (track_->GetIndentationLevel() > 1) {
    ORBIT_LOG_ONCE("Error: Track indentation level is greater than one, layout will be broken.");
  }
  return track_->GetIndentationLevel() > 0 ? layout_->GetTextBoxHeight() : 0.f;
}

void TrackHeader::OnDrag(int x, int y) {
  CaptureViewElement::OnDrag(x, y);

  if (track_->Draggable()) {
    track_->DragBy(mouse_pos_cur_[1] - picking_offset_[1] - GetPos()[1]);
  }
}

bool TrackHeader::Draggable() { return track_->Draggable(); }

void TrackHeader::SetHeight(float height) {
  if (height == height_) return;
  height_ = height;
  RequestUpdate(RequestUpdateScope::kDraw);
}

}  // namespace orbit_gl
