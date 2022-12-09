// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MOCK_BATCHER_H_
#define ORBIT_GL_MOCK_BATCHER_H_

#include <absl/container/btree_map.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <set>
#include <vector>

#include "OrbitGl/Batcher.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/PickingManager.h"
#include "absl/container/btree_map.h"

namespace orbit_gl {

class MockBatcher : public Batcher {
 public:
  explicit MockBatcher(BatcherId batcher_id = BatcherId::kTimeGraph);
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& /*picking_color*/,
               std::unique_ptr<PickingUserData> /*user_data*/) override;
  void AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
              const Color& /*picking_color*/,
              std::unique_ptr<PickingUserData> /*user_data*/) override;
  void AddTriangle(const Triangle& triangle, float z, const std::array<Color, 3>& colors,
                   const Color& /*picking_color*/,
                   std::unique_ptr<PickingUserData> /*user_data*/) override;

  void ResetElements() override;
  [[nodiscard]] uint32_t GetNumElements() const override;

  [[nodiscard]] std::vector<float> GetLayers() const override {
    return std::vector<float>(z_layers_.begin(), z_layers_.end());
  }

  void DrawLayer(float /*layer*/, bool /*picking*/) override {}

  [[nodiscard]] const PickingUserData* GetUserData(PickingId /*id*/) const override {
    return nullptr;
  }

  [[nodiscard]] int GetNumLinesByColor(Color color) const {
    return num_lines_by_color_.contains(color) ? num_lines_by_color_.at(color) : 0;
  }
  [[nodiscard]] int GetNumLines() const;
  [[nodiscard]] int GetNumTriangles() const;
  [[nodiscard]] int GetNumBoxes() const;

  [[nodiscard]] bool IsEverythingInsideRectangle(const Vec2& start, const Vec2& size) const;
  [[nodiscard]] bool IsEverythingBetweenZLayers(float z_layer_min, float z_layer_max) const;

 private:
  void AdjustDrawingBoundaries(Vec2 point);

  Vec2 min_point_;
  Vec2 max_point_;
  std::set<float> z_layers_;
  int num_vertical_lines_ = 0;
  int num_horizontal_lines_ = 0;
  absl::btree_map<Color, int> num_lines_by_color_;
  absl::btree_map<Color, int> num_triangles_by_color_;
  absl::btree_map<Color, int> num_boxes_by_color_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MOCK_BATCHER_H_
