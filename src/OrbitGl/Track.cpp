// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Track.h"

#include <math.h>
#include <stddef.h>

#include <limits>

#include "AccessibleTrack.h"
#include "CoreMath.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitClientModel/CaptureData.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"

Track::Track(CaptureViewElement* parent, TimeGraph* time_graph, TimeGraphLayout* layout,
             const CaptureData* capture_data)
    : CaptureViewElement(parent, time_graph, layout),
      collapse_toggle_(std::make_shared<TriangleToggle>(
          TriangleToggle::State::kExpanded,
          [this](TriangleToggle::State state) { OnCollapseToggle(state); }, time_graph, layout,
          this)),
      layout_(layout),
      capture_data_(capture_data) {
  const Color kDarkGrey(50, 50, 50, 255);
  color_ = kDarkGrey;
  num_timers_ = 0;
  min_time_ = std::numeric_limits<uint64_t>::max();
  max_time_ = std::numeric_limits<uint64_t>::min();
  num_prioritized_trailing_characters_ = 0;
  thread_id_ = -1;
  process_id_ = -1;
  SetPinned(false);
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
    float x_rotated = cos_r * point[0] - sin_r * point[1];
    float y_rotated = sin_r * point[0] + cos_r * point[1];
    result.emplace_back(x_rotated, y_rotated);
  }
  return result;
}

void Track::DrawTriangleFan(Batcher* batcher, const std::vector<Vec2>& points, const Vec2& pos,
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
    batcher->AddTriangle(triangle, color, shared_from_this());
  }
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> Track::CreateAccessibleInterface() {
  return std::make_unique<orbit_gl::AccessibleTrack>(this, layout_);
}

void Track::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  CaptureViewElement::Draw(canvas, picking_mode, z_offset);

  Batcher* ui_batcher = canvas->GetBatcher();
  const bool picking = picking_mode != PickingMode::kNone;

  float x0 = pos_[0];
  float y0 = pos_[1];
  float track_z = GlCanvas::kZValueTrack + z_offset;
  float text_z = GlCanvas::kZValueTrackText + z_offset;
  float top_margin = layout_->GetTrackTopMargin();

  const Color kBackgroundColor = GetBackgroundColor();

  // Draw tab.
  float label_height = layout_->GetTrackTabHeight();
  float half_label_height = 0.5f * label_height;
  float label_width = layout_->GetTrackTabWidth();
  float half_label_width = 0.5f * label_width;
  float tab_x0 = x0 + layout_->GetTrackTabOffset();

  Box box(Vec2(tab_x0, y0), Vec2(label_width, -label_height), track_z);
  ui_batcher->AddBox(box, kBackgroundColor, shared_from_this());

  // Draw rounded corners.
  if (!picking) {
    float right_margin = time_graph_->GetRightMargin();
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
    Vec2 top_left(tab_x0, y0);
    Vec2 tab_top_right(top_left[0] + label_width, top_left[1]);
    Vec2 tab_bottom_right(top_left[0] + label_width, top_left[1] - label_height + top_margin);
    Vec2 content_bottom_left(top_left[0] - tab_x0, top_left[1] - size_[1]);
    Vec2 content_bottom_right(top_left[0] + size_[0] - right_margin, top_left[1] - size_[1]);
    Vec2 content_top_right(top_left[0] + size_[0] - right_margin,
                           top_left[1] - label_height + top_margin);

    DrawTriangleFan(ui_batcher, rounded_corner, top_left, GlCanvas::kBackgroundColor, -90.f,
                    track_z);
    DrawTriangleFan(ui_batcher, rounded_corner, tab_top_right, GlCanvas::kBackgroundColor, 180.f,
                    track_z);
    DrawTriangleFan(ui_batcher, rounded_corner, tab_bottom_right, kBackgroundColor, 0, track_z);
    DrawTriangleFan(ui_batcher, rounded_corner, content_bottom_left, GlCanvas::kBackgroundColor, 0,
                    track_z);
    DrawTriangleFan(ui_batcher, rounded_corner, content_bottom_right, GlCanvas::kBackgroundColor,
                    90.f, track_z);
    DrawTriangleFan(ui_batcher, rounded_corner, content_top_right, GlCanvas::kBackgroundColor,
                    180.f, track_z);
  }

  // Collapse toggle state management.
  if (!this->IsCollapsible()) {
    collapse_toggle_->SetState(TriangleToggle::State::kInactive);
  } else if (collapse_toggle_->IsInactive()) {
    collapse_toggle_->ResetToInitialState();
  }

  // Draw collapsing triangle.
  float button_offset = layout_->GetCollapseButtonOffset();
  float toggle_y_pos = pos_[1] - half_label_height;
  Vec2 toggle_pos = Vec2(tab_x0 + button_offset, toggle_y_pos);
  collapse_toggle_->SetPos(toggle_pos[0], toggle_pos[1]);
  collapse_toggle_->Draw(canvas, picking_mode, z_offset);

  // Draw label.
  if (!picking) {
    uint32_t font_size = layout_->CalculateZoomedFontSize();
    auto& text_renderer = canvas->GetTextRenderer();
    float label_offset_x = layout_->GetTrackLabelOffsetX();
    float label_offset_y = text_renderer.GetStringHeight("o", font_size) / 2.f;

    const Color kColor =
        IsTrackSelected() ? GlCanvas::kTabTextColorSelected : Color(255, 255, 255, 255);

    text_renderer.AddTextTrailingCharsPrioritized(
        label_.c_str(), tab_x0 + label_offset_x, toggle_y_pos - label_offset_y, text_z, kColor,
        GetNumberOfPrioritizedTrailingCharacters(), font_size, label_width - label_offset_x);
  }

  // Draw track's content background.
  if (!picking) {
    if (layout_->GetDrawTrackBackground()) {
      Box box(Vec2(x0, y0 - label_height + top_margin),
              Vec2(size_[0], -size_[1] + label_height - top_margin), track_z);
      ui_batcher->AddBox(box, kBackgroundColor, shared_from_this());
    }
  }
}

void Track::UpdatePrimitives(Batcher* /*batcher*/, uint64_t /*t_min*/, uint64_t /*t_max*/,
                             PickingMode /*  picking_mode*/, float /*z_offset*/) {}

void Track::SetPinned(bool value) { pinned_ = value; }

Color Track::GetBackgroundColor() const {
  int32_t capture_process_id = capture_data_ ? capture_data_->process_id() : -1;

  if (process_id_ != -1 && process_id_ != capture_process_id) {
    const Color kExternalProcessColor(30, 30, 40, 255);
    return kExternalProcessColor;
  }

  return color_;
}

void Track::OnCollapseToggle(TriangleToggle::State /*state*/) {
  time_graph_->RequestUpdatePrimitives();
}

void Track::OnDrag(int x, int y) {
  CaptureViewElement::OnDrag(x, y);

  pos_[1] = mouse_pos_cur_[1] - picking_offset_[1];
  time_graph_->VerticallyMoveIntoView(*this);
}
