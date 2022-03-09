// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_IMPL_H_
#define ORBIT_GL_BATCHER_IMPL_H_

#include <Batcher.h>

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Containers/BlockChain.h"
#include "PickingManager.h"

struct LineBuffer {
  void Reset() {
    lines_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_LINES_PER_BLOCK = 64 * 1024;
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

  static const int NUM_BOXES_PER_BLOCK = 64 * 1024;
  orbit_containers::BlockChain<Box, NUM_BOXES_PER_BLOCK> boxes_;
  orbit_containers::BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> colors_;
  orbit_containers::BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> picking_colors_;
};

struct TriangleBuffer {
  void Reset() {
    triangles_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_TRIANGLES_PER_BLOCK = 64 * 1024;
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
};

/**
Implements internal methods to collects primitives to be rendered at a later point in time.

NOTE: The BatcherImpl assumes x/y coordinates are in pixels and will automatically round those
down to the next integer in all Batcher::AddXXX methods. This fixes the issue of primitives
"jumping" around when their coordinates are changed slightly.
**/
class BatcherImpl : public Batcher {
 public:
  BatcherImpl(BatcherId batcher_id, PickingManager* picking_manager = nullptr)
      : Batcher(batcher_id, picking_manager) {}
  [[nodiscard]] std::vector<float> GetLayers() const override;
  void DrawLayer(float layer, bool picking) const override;

 protected:
  std::unordered_map<float, PrimitiveBuffers> primitive_buffers_by_layer_;

 private:
  void ResetElements() override;
  void AddLineInternal(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
                       std::unique_ptr<PickingUserData> user_data) override;
  void AddBoxInternal(const Box& box, const std::array<Color, 4>& colors,
                      const Color& picking_color,
                      std::unique_ptr<PickingUserData> user_data) override;
  void AddTriangleInternal(const Triangle& triangle, const std::array<Color, 3>& colors,
                           const Color& picking_color,
                           std::unique_ptr<PickingUserData> user_data) override;

  void DrawLineBuffer(float layer, bool picking) const;
  void DrawBoxBuffer(float layer, bool picking) const;
  void DrawTriangleBuffer(float layer, bool picking) const;
};

#endif  // ORBIT_GL_BATCHER_IMPL_H_
