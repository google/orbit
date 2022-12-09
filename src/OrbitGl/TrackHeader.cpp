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
#include "OrbitGl/TrackRenderHelper.h"

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
  const float track_z = GlCanvas::kZValueTrack;
  const float text_z = GlCanvas::kZValueTrackText;

  // Draw tab background
  const float label_height = GetHeight();
  const float half_label_height = 0.5f * label_height;
  const float label_width = GetWidth();
  const float half_label_width = 0.5f * label_width;

  const float indentation_x0 =
      x0 + (track_->GetIndentationLevel() * layout_->GetTrackIndentOffset());
  Quad box = MakeBox(Vec2(indentation_x0, y0), Vec2(label_width, label_height));
  primitive_assembler.AddBox(box, track_z, track_->GetTrackBackgroundColor(), shared_from_this());

  // Early-out: In picking mode, don't draw the text and rounded corners.
  if (picking) {
    return;
  }

  // Don't draw rounded corners when this track is a child of another track
  if (track_->GetIndentationLevel() == 0) {
    // Draw rounded corners.
    float radius = std::min(layout_->GetRoundingRadius(), half_label_height);
    radius = std::min(radius, half_label_width);
    auto sides = static_cast<uint32_t>(layout_->GetRoundingNumSides() + 0.5f);
    auto rounded_corner = GetRoundedCornerMask(radius, sides);

    // This only draw the tab-part of a track. It's expecting to sit on the top of the track.
    // See comments in Track::DoDraw for a full picture.
    //
    // top_left       tab_top_right
    //  __________________
    // /                   `
    // |___________________(
    // [ Track content below]

    Vec2 top_left(indentation_x0, y0);
    Vec2 tab_top_right(top_left[0] + label_width, top_left[1]);
    Vec2 tab_bottom_right(top_left[0] + label_width, top_left[1] + label_height);

    auto shared_this = shared_from_this();
    DrawTriangleFan(primitive_assembler, rounded_corner, top_left, GlCanvas::kBackgroundColor, 90.f,
                    track_z, shared_this);
    DrawTriangleFan(primitive_assembler, rounded_corner, tab_top_right, GlCanvas::kBackgroundColor,
                    180.f, track_z, shared_this);
    DrawTriangleFan(primitive_assembler, rounded_corner, tab_bottom_right,
                    track_->GetTrackBackgroundColor(), 0, track_z, shared_this);
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
      track_->GetLabel().c_str(), indentation_x0 + label_offset_x, y0 + half_label_height, text_z,
      formatting, track_->GetNumberOfPrioritizedTrailingCharacters());
}

void orbit_gl::TrackHeader::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  UpdateCollapseToggle();
}

void TrackHeader::UpdateCollapseToggle() {
  const float label_height = layout_->GetTrackTabHeight();
  const float half_label_height = 0.5f * label_height;
  const float x0 = GetPos()[0] + layout_->GetTrackTabOffset() +
                   layout_->GetTrackIndentOffset() * track_->GetIndentationLevel();
  const float button_offset = layout_->GetCollapseButtonOffset();
  const float size = layout_->GetCollapseButtonSize(track_->GetIndentationLevel());
  const float toggle_y_pos = GetPos()[1] + half_label_height - size / 2;
  Vec2 toggle_pos = Vec2(x0 + button_offset, toggle_y_pos);

  collapse_toggle_->SetWidth(size);
  collapse_toggle_->SetHeight(size);
  collapse_toggle_->SetPos(toggle_pos[0], toggle_pos[1]);

  // Update the "collapsible" property of the triangle toggle to match the same property in the
  // parent track. This makes sure that changes to the track "collapsible" property are correctly
  // disabling the triangle toggle, even if they change during runtime.
  collapse_toggle_->SetIsCollapsible(track_->IsCollapsible());
}

void TrackHeader::OnDrag(int x, int y) {
  CaptureViewElement::OnDrag(x, y);

  if (track_->Draggable()) {
    track_->DragBy(mouse_pos_cur_[1] - picking_offset_[1] - GetPos()[1]);
  }
}

bool TrackHeader::Draggable() { return track_->Draggable(); }

}  // namespace orbit_gl