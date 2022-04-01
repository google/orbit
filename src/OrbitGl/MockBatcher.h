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
  explicit MockBatcher();
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& /*picking_color*/,
               std::unique_ptr<PickingUserData> /*user_data*/) override;
  void AddBox(const Box& box, const std::array<Color, 4>& colors, const Color& /*picking_color*/,
              std::unique_ptr<PickingUserData> /*user_data*/) override;
  void AddTriangle(const Triangle& triangle, const std::array<Color, 3>& colors,
                   const Color& /*picking_color*/,
                   std::unique_ptr<PickingUserData> /*user_data*/) override;

  void ResetElements() override;
  [[nodiscard]] uint32_t GetNumElements() const override;

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
  [[nodiscard]] int GetNumLines() const;
  [[nodiscard]] int GetNumTriangles() const;
  [[nodiscard]] int GetNumBoxes() const;

  [[nodiscard]] bool IsEverythingInsideRectangle(Vec2 start, Vec2 end) const;
  [[nodiscard]] bool IsEverythingBetweenZLayers(float z_layer_min, float z_layer_max) const;

 private:
  void AdjustDrawingBoundaries(Vec3 point);

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
