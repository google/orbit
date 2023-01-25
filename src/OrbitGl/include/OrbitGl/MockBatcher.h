// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MOCK_BATCHER_H_
#define ORBIT_GL_MOCK_BATCHER_H_

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_set.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/Batcher.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/PickingManager.h"
#include "absl/container/flat_hash_set.h"

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

  [[nodiscard]] std::vector<BatchRenderGroupId> GetNonEmptyRenderGroups() const override {
    return {render_groups_.begin(), render_groups_.end()};
  }

  void DrawRenderGroup(const BatchRenderGroupId& /*group*/, bool /*picking*/) override {}

  [[nodiscard]] Statistics GetStatistics() const override { return {}; }

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
  absl::flat_hash_set<BatchRenderGroupId> render_groups_;
  int num_vertical_lines_ = 0;
  int num_horizontal_lines_ = 0;
  absl::btree_map<Color, int> num_lines_by_color_;
  absl::btree_map<Color, int> num_triangles_by_color_;
  absl::btree_map<Color, int> num_boxes_by_color_;

  BatchRenderGroupStateManager owned_manager_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MOCK_BATCHER_H_
