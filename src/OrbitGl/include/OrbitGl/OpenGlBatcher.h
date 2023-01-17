// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_OPEN_GL_BATCHER_H_
#define ORBIT_GL_OPEN_GL_BATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include <QOpenGLFunctions>
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Containers/BlockChain.h"
#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/Batcher.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

namespace orbit_gl_internal {

struct LineBuffer {
  void Reset() {
    lines_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_LINES_PER_BLOCK = 1024;
  orbit_containers::BlockChain<Line, NUM_LINES_PER_BLOCK> lines_;
  orbit_containers::BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> colors_;
  orbit_containers::BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> picking_colors_;
};

struct BoxBuffer {
  void Reset() {
    boxes_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_BOXES_PER_BLOCK = 1024;
  orbit_containers::BlockChain<Quad, NUM_BOXES_PER_BLOCK> boxes_;
  orbit_containers::BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> colors_;
  orbit_containers::BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> picking_colors_;
};

struct TriangleBuffer {
  void Reset() {
    triangles_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_TRIANGLES_PER_BLOCK = 1024;
  orbit_containers::BlockChain<Triangle, NUM_TRIANGLES_PER_BLOCK> triangles_;
  orbit_containers::BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> colors_;
  orbit_containers::BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> picking_colors_;
};

struct PrimitiveBuffers {
  void Reset() {
    line_buffer.Reset();
    box_buffer.Reset();
    triangle_buffer.Reset();
  }

  LineBuffer line_buffer;
  BoxBuffer box_buffer;
  TriangleBuffer triangle_buffer;
  BatchRenderGroupState metadata;
};

}  // namespace orbit_gl_internal

// Implements internal methods to collects primitives to be rendered at a later point in time.
//
// NOTE: The OpenGlBatcher assumes x/y coordinates are in pixels and will automatically round those
// down to the next integer in all Batcher::AddXXX methods. This fixes the issue of primitives
// "jumping" around when their coordinates are changed slightly.
class OpenGlBatcher : public Batcher, protected QOpenGLFunctions {
 public:
  explicit OpenGlBatcher(BatchRenderGroupManager* manager, BatcherId batcher_id)
      : Batcher(manager, batcher_id) {}

  void ResetElements() override;
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
               std::unique_ptr<PickingUserData> user_data = nullptr) override;
  void AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
              const Color& picking_color,
              std::unique_ptr<PickingUserData> user_data = nullptr) override;
  void AddTriangle(const Triangle& triangle, float z, const std::array<Color, 3>& colors,
                   const Color& picking_color,
                   std::unique_ptr<PickingUserData> user_data = nullptr) override;

  [[nodiscard]] uint32_t GetNumElements() const override { return user_data_.size(); }
  [[nodiscard]] std::vector<BatchRenderGroupId> GetNonEmptyRenderGroups() const override;
  void DrawRenderGroup(const BatchRenderGroupId& group, bool picking) override;

  [[nodiscard]] const PickingUserData* GetUserData(PickingId id) const override;

  [[nodiscard]] Statistics GetStatistics() const override;

 protected:
  std::unordered_map<BatchRenderGroupId, orbit_gl_internal::PrimitiveBuffers>
      primitive_buffers_by_group_;
  std::vector<std::unique_ptr<PickingUserData>> user_data_;

 private:
  void DrawLineBuffer(const BatchRenderGroupId& group, bool picking);
  void DrawBoxBuffer(const BatchRenderGroupId& group, bool picking);
  void DrawTriangleBuffer(const BatchRenderGroupId& group, bool picking);

  inline void UpdateRenderGroupZ(float layer_z_value) {
    // This should be refactored in the future:
    // Different z-values mean different render groups. Adjusting the z-value of the current render
    // group should be done explicitly by the users of this class instead of implicitly here.
    // However, this would mean a lot of changes to all the rendering code, we'd remove all
    // z-parameters from all "AddXXX" calls and instead rely on a stateful "current z" setting in
    // the batcher.
    if (current_render_group_.layer != layer_z_value) {
      // We also need to copy over the state to the new group, because users of this class assume to
      // set the state once and keep it until they explicitly change the render group.
      auto current_group_state = manager_->GetGroupState(current_render_group_);
      current_render_group_.layer = layer_z_value;
      manager_->SetGroupState(current_render_group_, current_group_state);
    }
  }
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_OPEN_GL_BATCHER_H_
