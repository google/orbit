// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/MockBatcher.h"

#include <GteVector.h>
#include <absl/container/btree_map.h>

#include <limits>

#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

MockBatcher::MockBatcher(BatcherId batcher_id) : Batcher(batcher_id) { ResetElements(); }

void MockBatcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                          const Color& /*picking_color*/,
                          std::unique_ptr<PickingUserData> /*user_data*/) {
  num_lines_by_color_[color]++;
  if (from[0] == to[0]) num_vertical_lines_++;
  if (from[1] == to[1]) num_horizontal_lines_++;
  AdjustDrawingBoundaries({from[0], from[1]});
  AdjustDrawingBoundaries({to[0], to[1]});
  z_layers_.insert(z);
}
void MockBatcher::AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
                         const Color& /*picking_color*/,
                         std::unique_ptr<PickingUserData> /*user_data*/) {
  num_boxes_by_color_[colors[0]]++;
  for (int i = 0; i < 4; i++) {
    AdjustDrawingBoundaries(box.vertices[i]);
  }
  z_layers_.insert(z);
}
void MockBatcher::AddTriangle(const Triangle& triangle, float z, const std::array<Color, 3>& colors,
                              const Color& /*picking_color*/,
                              std::unique_ptr<PickingUserData> /*user_data*/) {
  num_triangles_by_color_[colors[0]]++;
  for (int i = 0; i < 3; i++) {
    AdjustDrawingBoundaries(triangle.vertices[i]);
  }
  z_layers_.insert(z);
}

void MockBatcher::ResetElements() {
  num_lines_by_color_.clear();
  num_triangles_by_color_.clear();
  num_boxes_by_color_.clear();
  num_horizontal_lines_ = 0;
  num_vertical_lines_ = 0;
  min_point_ = Vec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
  max_point_ = Vec2{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
  z_layers_.clear();
}

uint32_t MockBatcher::GetNumElements() const {
  return GetNumLines() + GetNumBoxes() + GetNumTriangles();
}

int MockBatcher::GetNumLines() const {
  int total_lines = 0;
  for (const auto& [unused_color, num_lines] : num_lines_by_color_) {
    total_lines += num_lines;
  }
  return total_lines;
}

int MockBatcher::GetNumTriangles() const {
  int total_triangles = 0;
  for (const auto& [unused_color, num_triangles] : num_triangles_by_color_) {
    total_triangles += num_triangles;
  }
  return total_triangles;
}

int MockBatcher::GetNumBoxes() const {
  int total_boxes = 0;
  for (const auto& [unused_color, num_boxes] : num_boxes_by_color_) {
    total_boxes += num_boxes;
  }
  return total_boxes;
}

// To check that everything is inside a rectangle, we just need to check the minimum and maximum
// used coordinates.
bool MockBatcher::IsEverythingInsideRectangle(const Vec2& start, const Vec2& size) const {
  if (GetNumElements() == 0) return true;
  return IsInsideRectangle(min_point_, start, size) && IsInsideRectangle(max_point_, start, size);
}

bool MockBatcher::IsEverythingBetweenZLayers(float z_layer_min, float z_layer_max) const {
  return std::find_if_not(z_layers_.begin(), z_layers_.end(),
                          [z_layer_min, z_layer_max](float layer) {
                            return ClosedInterval<float>{z_layer_min, z_layer_max}.Contains(layer);
                          }) == z_layers_.end();
}

void MockBatcher::AdjustDrawingBoundaries(Vec2 point) {
  min_point_[0] = std::min(point[0], min_point_[0]);
  min_point_[1] = std::min(point[1], min_point_[1]);
  max_point_[0] = std::max(point[0], max_point_[0]);
  max_point_[1] = std::max(point[1], max_point_[1]);
}

}  // namespace orbit_gl