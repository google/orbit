// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Track.h"

#include <math.h>
#include <stddef.h>

#include "AccessibleTrack.h"
#include "ClientData/CaptureData.h"
#include "CoreMath.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/ThreadUtils.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

using orbit_client_data::TimerData;

Track::Track(CaptureViewElement* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
             orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
             const orbit_client_data::CaptureData* capture_data)
    : CaptureViewElement(parent, viewport, layout),
      pinned_{false},
      layout_(layout),
      timeline_info_(timeline_info),
      capture_data_(capture_data) {
  collapse_toggle_ = std::make_shared<TriangleToggle>(
      [this](bool is_collapsed) { OnCollapseToggle(is_collapsed); }, viewport, layout, this);
}

std::vector<Vec2> GetRoundedCornerMask(float radius, uint32_t num_sides) {
  std::vector<Vec2> points;
  points.emplace_back(0.f, 0.f);
  points.emplace_back(0.f, radius);

  float increment_radians = 0.5f * kPiFloat / static_cast<float>(num_sides);
  for (uint32_t i = 1; i < num_sides; ++i) {
    float angle = kPiFloat + static_cast<float>(i) * increment_radians;
    points.emplace_back(radius * cosf(angle) + radius, radius * sinf(angle) + radius);
  }

  points.emplace_back(radius, 0.f);
  return points;
}

std::vector<Vec2> RotatePoints(const std::vector<Vec2>& points, float rotation) {
  float cos_r = cosf(kPiFloat * rotation / 180.f);
  float sin_r = sinf(kPiFloat * rotation / 180.f);
  std::vector<Vec2> result;
  for (const Vec2& point : points) {
    float x_rotated = cos_r * point[0] + sin_r * point[1];
    float y_rotated = sin_r * point[0] - cos_r * point[1];
    result.emplace_back(x_rotated, y_rotated);
  }
  return result;
}

void Track::DrawTriangleFan(Batcher& batcher, const std::vector<Vec2>& points, const Vec2& pos,
                            const Color& color, float rotation, float z) {
  if (points.size() < 3) {
    return;
  }

  std::vector<Vec2> rotated_points = RotatePoints(points, rotation);
  Vec3 position(pos[0], pos[1], z);
  Vec3 pivot = position + Vec3(rotated_points[0][0], rotated_points[0][1], z);

  Vec3 vertices[2];
  vertices[0] = position + Vec3(rotated_points[1][0], rotated_points[1][1], z);

  for (size_t i = 1; i < rotated_points.size() - 1; ++i) {
    vertices[i % 2] = position + Vec3(rotated_points[i + 1][0], rotated_points[i + 1][1], z);
    Triangle triangle(pivot, vertices[i % 2], vertices[(i + 1) % 2]);
    batcher.AddTriangle(triangle, color, shared_from_this());
  }
}

void Track::UpdatePositionOfCollapseToggle() {
  const float label_height = layout_->GetTrackTabHeight();
  const float half_label_height = 0.5f * label_height;
  const float x0 = GetPos()[0] + layout_->GetTrackTabOffset() +
                   layout_->GetTrackIndentOffset() * indentation_level_;
  const float button_offset = layout_->GetCollapseButtonOffset();
  const float toggle_y_pos = GetPos()[1] + half_label_height;
  Vec2 toggle_pos = Vec2(x0 + button_offset, toggle_y_pos);

  float size = layout_->GetCollapseButtonSize(indentation_level_);
  collapse_toggle_->SetWidth(size);
  collapse_toggle_->SetHeight(size);
  collapse_toggle_->SetPos(toggle_pos[0], toggle_pos[1]);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> Track::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleTrack>(this, layout_);
}

void Track::DoDraw(Batcher& batcher, TextRenderer& text_renderer, const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(batcher, text_renderer, draw_context);

  if (headless_) return;

  const bool picking = draw_context.picking_mode != PickingMode::kNone;

  const float x0 = GetPos()[0];
  const float y0 = GetPos()[1];
  float track_z = GlCanvas::kZValueTrack;
  float text_z = GlCanvas::kZValueTrackText;

  Color track_background_color = GetTrackBackgroundColor();
  if (!draw_background_) {
    track_background_color[3] = 0;
  }

  // Draw tab.
  float label_height = layout_->GetTrackTabHeight();
  float half_label_height = 0.5f * label_height;
  float label_width = layout_->GetTrackTabWidth();
  float half_label_width = 0.5f * label_width;
  float tab_x0 = x0 + layout_->GetTrackTabOffset();

  const float indentation_x0 = tab_x0 + (indentation_level_ * layout_->GetTrackIndentOffset());
  Box box(Vec2(indentation_x0, y0), Vec2(label_width, label_height), track_z);
  batcher.AddBox(box, track_background_color, shared_from_this());

  Vec2 track_size = GetSize();

  // Draw rounded corners.
  if (!picking && draw_background_) {
    float radius = std::min(layout_->GetRoundingRadius(), half_label_height);
    radius = std::min(radius, half_label_width);
    uint32_t sides = static_cast<uint32_t>(layout_->GetRoundingNumSides() + 0.5f);
    auto rounded_corner = GetRoundedCornerMask(radius, sides);

    // top_left       tab_top_right
    //  __________________              content_top_right
    // /                  \_______________
    // |             tab_bottom_right     `
    // |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|
    // \__________________________________/
    // content_bottom_left                      content_bottom_right
    //
    // In addition, there is a small margin before and after the track content
    // defined by TrackContentTopMargin and TrackContentBottomMargin.
    // Both margins are factored into the total height of each track.
    Vec2 top_left(indentation_x0, y0);
    Vec2 tab_top_right(top_left[0] + label_width, top_left[1]);
    Vec2 tab_bottom_right(top_left[0] + label_width, top_left[1] + label_height);
    Vec2 content_bottom_left(top_left[0] - tab_x0, top_left[1] + track_size[1]);
    Vec2 content_bottom_right(top_left[0] + track_size[0], top_left[1] + track_size[1]);
    Vec2 content_top_right(top_left[0] + track_size[0], top_left[1] + label_height);

    DrawTriangleFan(batcher, rounded_corner, top_left, GlCanvas::kBackgroundColor, 90.f, track_z);
    DrawTriangleFan(batcher, rounded_corner, tab_top_right, GlCanvas::kBackgroundColor, 180.f,
                    track_z);
    DrawTriangleFan(batcher, rounded_corner, tab_bottom_right, track_background_color, 0, track_z);
    DrawTriangleFan(batcher, rounded_corner, content_bottom_left, GlCanvas::kBackgroundColor, 0,
                    track_z);
    DrawTriangleFan(batcher, rounded_corner, content_bottom_right, GlCanvas::kBackgroundColor,
                    -90.f, track_z);
    DrawTriangleFan(batcher, rounded_corner, content_top_right, GlCanvas::kBackgroundColor, 180.f,
                    track_z);
  }

  // Collapse toggle state management.
  collapse_toggle_->SetIsCollapsible(this->IsCollapsible());

  // Draw label.
  if (!picking) {
    uint32_t font_size = layout_->CalculateZoomedFontSize();
    // For the first 5 indentations, we decrease the font_size by 10 percent points (per
    // indentation).
    constexpr uint32_t kMaxIndentationLevel = 5;
    uint32_t capped_indentation_level = std::min(indentation_level_, kMaxIndentationLevel);
    font_size = (font_size * (10 - capped_indentation_level)) / 10;
    float label_offset_x = layout_->GetTrackLabelOffsetX();

    const Color kColor =
        IsTrackSelected() ? GlCanvas::kTabTextColorSelected : Color(255, 255, 255, 255);

    TextRenderer::TextFormatting formatting{font_size, kColor, label_width - label_offset_x};
    formatting.valign = TextRenderer::VAlign::Middle;

    text_renderer.AddTextTrailingCharsPrioritized(
        GetLabel().c_str(), indentation_x0 + label_offset_x, y0 + half_label_height, text_z,
        formatting, GetNumberOfPrioritizedTrailingCharacters());
  }

  // Draw track's content background.
  if (!picking) {
    if (layout_->GetDrawTrackBackground()) {
      Box box(Vec2(x0, y0 + label_height), Vec2(GetWidth(), GetHeight() - label_height), track_z);
      batcher.AddBox(box, track_background_color, shared_from_this());
    }
  }
}

void Track::DoUpdateLayout() {
  CaptureViewElement::DoUpdateLayout();

  UpdatePositionOfSubtracks();
  UpdatePositionOfCollapseToggle();
}

void Track::SetPinned(bool value) { pinned_ = value; }

Color Track::GetTrackBackgroundColor() const {
  uint32_t capture_process_id = capture_data_->process_id();

  if (GetProcessId() != orbit_base::kInvalidProcessId && GetProcessId() != capture_process_id &&
      GetType() != Type::kSchedulerTrack) {
    const Color kExternalProcessColor(30, 30, 40, 255);
    return kExternalProcessColor;
  }

  const Color kDarkGrey(50, 50, 50, 255);
  return kDarkGrey;
}

void Track::OnCollapseToggle(bool /*is_collapsed*/) { RequestUpdate(); }

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

void Track::SetHeadless(bool value) {
  if (headless_ == value) return;

  headless_ = value;
  collapse_toggle_->SetVisible(!headless_);
  RequestUpdate();
}

void Track::SetIndentationLevel(uint32_t level) {
  if (level == indentation_level_) return;

  indentation_level_ = level;
  RequestUpdate();
}

void Track::OnDrag(int x, int y) {
  CaptureViewElement::OnDrag(x, y);

  SetPos(GetPos()[0], mouse_pos_cur_[1] - picking_offset_[1]);
}
