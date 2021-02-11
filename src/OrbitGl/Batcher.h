// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_H_
#define ORBIT_GL_BATCHER_H_

#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BlockChain.h"
#include "CoreMath.h"
#include "Geometry.h"
#include "PickingManager.h"
#include "TextBox.h"

using TooltipCallback = std::function<std::string(PickingId)>;

struct PickingUserData {
  TextBox* text_box_;
  TooltipCallback generate_tooltip_;
  const void* custom_data_ = nullptr;

  explicit PickingUserData(TextBox* text_box = nullptr, TooltipCallback generate_tooltip = nullptr)
      : text_box_(text_box), generate_tooltip_(std::move(generate_tooltip)) {}
};

struct LineBuffer {
  void Reset() {
    lines_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_LINES_PER_BLOCK = 64 * 1024;
  BlockChain<Line, NUM_LINES_PER_BLOCK> lines_;
  BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> colors_;
  BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> picking_colors_;
};

struct BoxBuffer {
  void Reset() {
    boxes_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_BOXES_PER_BLOCK = 64 * 1024;
  BlockChain<Box, NUM_BOXES_PER_BLOCK> boxes_;
  BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> colors_;
  BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> picking_colors_;
};

struct TriangleBuffer {
  void Reset() {
    triangles_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
  }

  static const int NUM_TRIANGLES_PER_BLOCK = 64 * 1024;
  BlockChain<Triangle, NUM_TRIANGLES_PER_BLOCK> triangles_;
  BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> colors_;
  BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> picking_colors_;
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

enum class ShadingDirection { kLeftToRight, kRightToLeft, kTopToBottom, kBottomToTop };

/**
Collects primitives to be rendered at a later point in time.

By calling Batcher::AddXXX, primitives are added to internal CPU buffers, and sorted
into layers formed by equal z-coordinates. Each layer can then be drawn seperately with
Batcher::DrawLayer(), or all layers can be drawn at once in their correct order using
Batcher::Draw():

NOTE: The Batcher assumes x/y coordinates are in pixels and will automatically round those
down to the next integer in all Batcher::AddXXX methods. This fixes the issue of primitives
"jumping" around when their coordinates are changed slightly.
**/
class Batcher {
 public:
  explicit Batcher(BatcherId batcher_id, PickingManager* picking_manager = nullptr)
      : batcher_id_(batcher_id), picking_manager_(picking_manager) {
    constexpr int32_t kSteps = 22;
    const float angle = (kPiFloat * 2.f) / kSteps;
    for (int32_t i = 1; i <= kSteps; i++) {
      float new_x = sinf(angle * i);
      float new_y = cosf(angle * i);
      circle_points.emplace_back(Vec2(new_x, new_y));
    }
  }

  Batcher() = delete;
  Batcher(const Batcher&) = delete;
  Batcher(Batcher&&) = delete;

  void AddLine(Vec2 from, Vec2 to, float z, const Color& color,
               std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                       std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, std::shared_ptr<Pickable> pickable);
  void AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                       std::shared_ptr<Pickable> pickable);

  void AddBox(const Box& box, const std::array<Color, 4>& colors,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& box, const Color& color,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& box, const Color& color, std::shared_ptr<Pickable> pickable);

  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    ShadingDirection shading_direction);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    std::unique_ptr<PickingUserData> user_data,
                    ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    std::shared_ptr<Pickable> pickable,
                    ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddRoundedBox(Vec2 pos, Vec2 size, float z, float radius, const Color& color,
                     float margin = 0);

  void AddBottomLeftRoundedCorner(Vec2 pos, float radius, float z, const Color& color);
  void AddTopLeftRoundedCorner(Vec2 pos, float radius, float z, const Color& color);
  void AddTopRightRoundedCorner(Vec2 pos, float radius, float z, const Color& color);
  void AddBottomRightRoundedCorner(Vec2 pos, float radius, float z, const Color& color);

  void AddTriangle(const Triangle& triangle, const Color& color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddTriangle(const Triangle& triangle, const Color& color,
                   std::shared_ptr<Pickable> pickable);

  void AddCircle(Vec2 position, float radius, float z, Color color);
  [[nodiscard]] std::vector<float> GetLayers() const;
  void DrawLayer(float layer, bool picking = false) const;
  virtual void Draw(bool picking = false) const;

  void ResetElements();
  void StartNewFrame();

  [[nodiscard]] PickingManager* GetPickingManager() const { return picking_manager_; }
  void SetPickingManager(PickingManager* picking_manager) { picking_manager_ = picking_manager; }

  [[nodiscard]] const PickingUserData* GetUserData(PickingId id) const;
  [[nodiscard]] PickingUserData* GetUserData(PickingId id);

  [[nodiscard]] const TextBox* GetTextBox(PickingId id) const;

  static constexpr uint32_t kNumArcSides = 16;

 protected:
  void DrawLineBuffer(float layer, bool picking) const;
  void DrawBoxBuffer(float layer, bool picking) const;
  void DrawTriangleBuffer(float layer, bool picking) const;

  void GetBoxGradientColors(const Color& color, std::array<Color, 4>* colors,
                            ShadingDirection shading_direction = ShadingDirection::kLeftToRight);

  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
               std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& box, const std::array<Color, 4>& colors, const Color& picking_color,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddTriangle(const Triangle& triangle, const Color& color, const Color& picking_color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);

  BatcherId batcher_id_;
  PickingManager* picking_manager_;
  std::unordered_map<float, PrimitiveBuffers> primitive_buffers_by_layer_;

  std::vector<std::unique_ptr<PickingUserData>> user_data_;

  std::vector<Vec2> circle_points;
};

#endif
