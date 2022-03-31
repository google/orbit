// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MOCK_BATCHER_H_
#define ORBIT_GL_MOCK_BATCHER_H_

#include <limits>

#include "BatcherInterface.h"

namespace orbit_gl {

class MockBatcher : public BatcherInterface {
 public:
  explicit MockBatcher() : BatcherInterface(BatcherId::kTimeGraph) { ResetElements(); }
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& /*picking_color*/,
               std::unique_ptr<PickingUserData> /*user_data*/ = nullptr) override {
    num_lines_by_color_[color]++;
    if (from[0] == to[0]) num_vertical_lines_++;
    if (from[1] == to[1]) num_horizontal_lines_++;
    ProcessPoint({from[0], from[1], z});
    ProcessPoint({to[0], to[1], z});
  }
  void AddBox(const Box& box, const std::array<Color, 4>& colors, const Color& /*picking_color*/,
              std::unique_ptr<PickingUserData> /*user_data*/ = nullptr) override {
    num_boxes_by_color_[colors[0]]++;
    for (int i = 0; i < 4; i++) {
      ProcessPoint(box.vertices[i]);
    }
  }
  void AddTriangle(const Triangle& triangle, const std::array<Color, 3>& colors,
                   const Color& /*picking_color*/,
                   std::unique_ptr<PickingUserData> /*user_data*/ = nullptr) override {
    num_triangles_by_color_[colors[0]]++;
    for (int i = 0; i < 3; i++) {
      ProcessPoint(triangle.vertices[i]);
    }
  }

  void ResetElements() override {
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

  [[nodiscard]] uint32_t GetNumElements() const override {
    return GetNumLines() + GetNumBoxes() + GetNumTriangles();
  }

  [[nodiscard]] std::vector<float> GetLayers() const override { return {}; };
  virtual void DrawLayer(float /*layer*/, bool /*picking*/) const override{};

  [[nodiscard]] const PickingUserData* GetUserData(PickingId /*id*/) const override {
    return nullptr;
  }

  [[nodiscard]] int GetNumLinesByColor(Color color) const { return num_lines_by_color_.at(color); }
  [[nodiscard]] int GetNumTrianglesByColor(Color color) const {
    return num_triangles_by_color_.at(color);
  }
  [[nodiscard]] int GetNumBoxesByColor(Color color) const { return num_boxes_by_color_.at(color); }
  [[nodiscard]] int GetNumHorizontalLines() const { return num_horizontal_lines_; }
  [[nodiscard]] int GetNumVerticalLines() const { return num_vertical_lines_; }
  [[nodiscard]] int GetNumLines() const {
    int total_lines = 0;
    for (auto& [unused_color, num_lines] : num_lines_by_color_) {
      total_lines += num_lines;
    }
    return total_lines;
  }
  [[nodiscard]] int GetNumTriangles() const {
    int total_triangles = 0;
    for (auto& [unused_color, num_triangles] : num_triangles_by_color_) {
      total_triangles += num_triangles;
    }
    return total_triangles;
  }
  [[nodiscard]] int GetNumBoxes() const {
    int total_boxes = 0;
    for (auto& [unused_color, num_boxes] : num_boxes_by_color_) {
      total_boxes += num_boxes;
    }
    return total_boxes;
  }
  [[nodiscard]] bool IsEverythingInsideRectangle(Vec2 start, Vec2 end) {
    return start[0] <= min_point_[0] && start[0] >= max_point_[0] && start[1] <= min_point_[1] &&
           start[1] >= max_point_[1] && end[0] <= min_point_[0] && end[0] >= max_point_[0] &&
           end[1] <= min_point_[1] && end[1] >= max_point_[1];
  }

  [[nodiscard]] bool IsBetweenZLayers(float z_layer_1, float z_layer_2) {
    return z_layer_1 <= min_point_[2] && z_layer_1 >= max_point_[2] && z_layer_2 <= min_point_[2] &&
           z_layer_2 >= max_point_[2];
  }

 private:
  void ProcessPoint(Vec3 point) {
    min_point_[0] = std::min(point[0], min_point_[0]);
    min_point_[1] = std::min(point[1], min_point_[1]);
    min_point_[2] = std::min(point[2], min_point_[2]);
    min_point_[0] = std::max(point[0], max_point_[0]);
    min_point_[0] = std::max(point[1], max_point_[1]);
    min_point_[0] = std::max(point[2], max_point_[2]);
  }
  Vec3 min_point_;
  Vec3 max_point_;
  int num_vertical_lines_;
  int num_horizontal_lines_;
  std::map<Color, int> num_lines_by_color_;
  std::map<Color, int> num_triangles_by_color_;
  std::map<Color, int> num_boxes_by_color_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MOCK_BATCHER_H_
