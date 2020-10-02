// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Track.h"

#include "CoreMath.h"
#include "GlCanvas.h"
#include "TimeGraphLayout.h"
#include "absl/strings/str_format.h"

Track::Track(TimeGraph* time_graph)
    : time_graph_(time_graph),
      collapse_toggle_(std::make_shared<TriangleToggle>(
          TriangleToggle::State::kExpanded,
          [this](TriangleToggle::State state) { OnCollapseToggle(state); }, time_graph)) {
  mouse_pos_[0] = mouse_pos_[1] = Vec2(0, 0);
  pos_ = Vec2(0, 0);
  size_ = Vec2(0, 0);
  picking_offset_ = Vec2(0, 0);
  picked_ = false;
  moving_ = false;
  canvas_ = nullptr;
  const Color kDarkGrey(50, 50, 50, 255);
  color_ = kDarkGrey;
  num_timers_ = 0;
  min_time_ = std::numeric_limits<uint64_t>::max();
  max_time_ = std::numeric_limits<uint64_t>::min();
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
    result.push_back(Vec2(x_rotated, y_rotated));
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

void Track::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  Batcher* batcher = canvas->GetBatcher();

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const bool picking = picking_mode != PickingMode::kNone;

  float x0 = pos_[0];
  float x1 = x0 + size_[0];
  float y0 = pos_[1];
  float y1 = y0 - size_[1];
  float track_z = GlCanvas::kZValueTrack;
  float text_z = GlCanvas::kZValueText;
  float top_margin = layout.GetTrackTopMargin();

  // Draw track background.
  if (!picking) {
    if (layout.GetDrawTrackBackground()) {
      Box box(Vec2(x0, y0 + top_margin), Vec2(size_[0], -size_[1] - top_margin), track_z);
      batcher->AddBox(box, GlCanvas::kTabColor, shared_from_this());
    }
  }

  // Draw tab.
  float label_height = layout.GetTrackTabHeight();
  float half_label_height = 0.5f * label_height;
  float label_width = layout.GetTrackTabWidth();
  float half_label_width = 0.5f * label_width;
  float tab_x0 = x0 + layout.GetTrackTabOffset();

  Box box(Vec2(tab_x0, y0), Vec2(label_width, label_height), track_z);
  batcher->AddBox(box, GlCanvas::kTabColor, shared_from_this());

  // Draw rounded corners.
  if (!picking) {
    float right_margin = time_graph_->GetRightMargin();
    float radius = std::min(layout.GetRoundingRadius(), half_label_height);
    radius = std::min(radius, half_label_width);
    uint32_t sides = static_cast<uint32_t>(layout.GetRoundingNumSides() + 0.5f);
    auto rounded_corner = GetRoundedCornerMask(radius, sides);
    Vec2 bottom_left(x0, y1);
    Vec2 bottom_right(tab_x0 + label_width, y0 + top_margin);
    Vec2 top_right(tab_x0 + label_width, y0 + label_height);
    Vec2 top_left(tab_x0, y0 + label_height);
    Vec2 end_bottom(x1 - right_margin, y1);
    Vec2 end_top(x1 - right_margin, y0 + top_margin);
    float z = GlCanvas::kZValueRoundingCorner;

    DrawTriangleFan(batcher, rounded_corner, bottom_left, GlCanvas::kBackgroundColor, 0, z);
    DrawTriangleFan(batcher, rounded_corner, bottom_right, GlCanvas::kTabColor, 0, z);
    DrawTriangleFan(batcher, rounded_corner, top_right, GlCanvas::kBackgroundColor, 180.f, z);
    DrawTriangleFan(batcher, rounded_corner, top_left, GlCanvas::kBackgroundColor, -90.f, z);
    DrawTriangleFan(batcher, rounded_corner, end_bottom, GlCanvas::kBackgroundColor, 90.f, z);
    DrawTriangleFan(batcher, rounded_corner, end_top, GlCanvas::kBackgroundColor, 180.f, z);
  }

  // Collapse toggle state management.
  if (!this->IsCollapsable()) {
    collapse_toggle_->SetState(TriangleToggle::State::kInactive);
  } else if (collapse_toggle_->IsInactive()) {
    collapse_toggle_->ResetToInitialState();
  }

  // Draw collapsing triangle.
  float button_offset = layout.GetCollapseButtonOffset();
  float toggle_y_pos = pos_[1] + half_label_height;
  Vec2 toggle_pos = Vec2(tab_x0 + button_offset, toggle_y_pos);
  collapse_toggle_->SetPos(toggle_pos);
  collapse_toggle_->Draw(canvas, picking_mode);

  if (!picking) {
    // Draw label.
    auto first_parenthesis = label_.find('(');
    auto first_brackets = label_.find('[');
    auto id_label_start = std::min(first_parenthesis, first_brackets);
    int trailing_characters =
        (id_label_start == label_.npos) ? 0 : (label_.size() - id_label_start);
    float label_offset_x = layout.GetTrackLabelOffsetX();
    // Vertical offset for the text to be aligned to the center of the triangle.
    float label_offset_y = GCurrentTimeGraph->GetFontSize() / 3.f;
    const Color kColor =
        IsTrackSelected() ? GlCanvas::kTabTextColorSelected : Color(255, 255, 255, 255);
    canvas->GetTextRenderer().AddTextTrailingCharsPrioritized(
        label_.c_str(), tab_x0 + label_offset_x, toggle_y_pos - label_offset_y, text_z, kColor,
        trailing_characters, time_graph_->CalculateZoomedFontSize(), label_width - label_offset_x);
  }

  canvas_ = canvas;
}

void Track::UpdatePrimitives(uint64_t /*t_min*/, uint64_t /*t_max*/,
                             PickingMode /*  picking_mode*/) {}

void Track::SetPos(float a_X, float a_Y) {
  if (!moving_) {
    pos_ = Vec2(a_X, a_Y);
  }
}

void Track::SetY(float y) {
  if (!moving_) {
    pos_[1] = y;
  }
}

void Track::SetSize(float a_SizeX, float a_SizeY) { size_ = Vec2(a_SizeX, a_SizeY); }

void Track::OnCollapseToggle(TriangleToggle::State /*state*/) {
  time_graph_->NeedsUpdate();
  time_graph_->NeedsRedraw();
}

void Track::OnPick(int a_X, int a_Y) {
  if (!picking_enabled_) return;

  Vec2& mousePos = mouse_pos_[0];
  canvas_->ScreenToWorld(a_X, a_Y, mousePos[0], mousePos[1]);
  picking_offset_ = mousePos - pos_;
  mouse_pos_[1] = mouse_pos_[0];
  picked_ = true;
}

void Track::OnRelease() {
  if (!picking_enabled_) return;

  picked_ = false;
  moving_ = false;
  time_graph_->NeedsUpdate();
}

void Track::OnDrag(int a_X, int a_Y) {
  if (!picking_enabled_) return;

  moving_ = true;
  float x = 0.f;
  canvas_->ScreenToWorld(a_X, a_Y, x, pos_[1]);
  mouse_pos_[1] = pos_;
  pos_[1] -= picking_offset_[1];
  time_graph_->NeedsUpdate();
}
