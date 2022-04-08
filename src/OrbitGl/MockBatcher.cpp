// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MockBatcher.h"

#include "CoreMath.h"

namespace orbit_gl {

[[nodiscard]] static bool IsInBetween(float point, float min, float max) {
  return point >= min && point <= max;
}

[[nodiscard]] static bool IsInsideRectangle(Vec2 point, Vec2 top_left, Vec2 bottom_right) {
  for (int i = 0; i < 2; i++) {
    if (!IsInBetween(point[i], top_left[i], bottom_right[i])) {
      return false;
    }
  }
  return true;
}

MockBatcher::MockBatcher(BatcherId batcher_id) : Batcher(batcher_id) { ResetElements(); }

void MockBatcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                          const Color& /*picking_color*/,
                          std::unique_ptr<PickingUserData> /*user_data*/) {
  num_lines_by_color_[color]++;
  if (from[0] == to[0]) num_vertical_lines_++;
  if (from[1] == to[1]) num_horizontal_lines_++;
  AdjustDrawingBoundaries({from[0], from[1], z});
  AdjustDrawingBoundaries({to[0], to[1], z});
}
void MockBatcher::AddBox(const Tetragon& box, const std::array<Color, 4>& colors,
                         const Color& /*picking_color*/,
                         std::unique_ptr<PickingUserData> /*user_data*/) {
  num_boxes_by_color_[colors[0]]++;
  for (int i = 0; i < 4; i++) {
    AdjustDrawingBoundaries(box.vertices[i]);
  }
}
void MockBatcher::AddTriangle(const Triangle& triangle, const std::array<Color, 3>& colors,
                              const Color& /*picking_color*/,
                              std::unique_ptr<PickingUserData> /*user_data*/) {
  num_triangles_by_color_[colors[0]]++;
  for (int i = 0; i < 3; i++) {
    AdjustDrawingBoundaries(triangle.vertices[i]);
  }
}

void MockBatcher::ResetElements() {
  num_lines_by_color_.clear();
  num_triangles_by_color_.clear();
  num_boxes_by_color_.clear();
  num_horizontal_lines_ = 0;
  num_vertical_lines_ = 0;
  min_point_ = Vec3{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max()};
  max_point_ = Vec3{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest()};
}

uint32_t MockBatcher::GetNumElements() const {
  return GetNumLines() + GetNumBoxes() + GetNumTriangles();
}

int MockBatcher::GetNumLines() const {
  int total_lines = 0;
  for (auto& [unused_color, num_lines] : num_lines_by_color_) {
    total_lines += num_lines;
  }
  return total_lines;
}

int MockBatcher::GetNumTriangles() const {
  int total_triangles = 0;
  for (auto& [unused_color, num_triangles] : num_triangles_by_color_) {
    total_triangles += num_triangles;
  }
  return total_triangles;
}

int MockBatcher::GetNumBoxes() const {
  int total_boxes = 0;
  for (auto& [unused_color, num_boxes] : num_boxes_by_color_) {
    total_boxes += num_boxes;
  }
  return total_boxes;
}

// To check that everything is inside a rectangle, we just need to check the minimum and maximum
// used coordinates.
bool MockBatcher::IsEverythingInsideRectangle(Vec2 start, Vec2 end) const {
  return IsInsideRectangle({min_point_[0], min_point_[1]}, start, end) &&
         IsInsideRectangle({max_point_[0], max_point_[1]}, start, end);
}

bool MockBatcher::IsEverythingBetweenZLayers(float z_layer_min, float z_layer_max) const {
  return IsInBetween(min_point_[2], z_layer_min, z_layer_max) &&
         IsInBetween(max_point_[2], z_layer_min, z_layer_max);
}

void MockBatcher::AdjustDrawingBoundaries(Vec3 point) {
  min_point_[0] = std::min(point[0], min_point_[0]);
  min_point_[1] = std::min(point[1], min_point_[1]);
  min_point_[2] = std::min(point[2], min_point_[2]);
  max_point_[0] = std::max(point[0], max_point_[0]);
  max_point_[1] = std::max(point[1], max_point_[1]);
  max_point_[2] = std::max(point[2], max_point_[2]);
}

}  // namespace orbit_gl