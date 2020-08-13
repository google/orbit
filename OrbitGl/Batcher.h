// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_H_
#define ORBIT_GL_BATCHER_H_

#include <utility>
#include <vector>

#include "BlockChain.h"
#include "Geometry.h"
#include "PickingManager.h"

using TooltipCallback = std::function<std::string(PickingId)>;

struct PickingUserData {
  TextBox* text_box_;
  TooltipCallback generate_tooltip_;
  void* custom_data_ = nullptr;

  explicit PickingUserData(TextBox* text_box = nullptr,
                           TooltipCallback generate_tooltip = nullptr)
      : text_box_(text_box), generate_tooltip_(std::move(generate_tooltip)) {}
};

struct LineBuffer {
  void Reset() {
    lines_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
    user_data_.clear();
  }

  static const int NUM_LINES_PER_BLOCK = 64 * 1024;
  BlockChain<Line, NUM_LINES_PER_BLOCK> lines_;
  BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> colors_;
  BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> picking_colors_;
  std::vector<std::unique_ptr<PickingUserData>> user_data_;
};

struct BoxBuffer {
  void Reset() {
    boxes_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
    user_data_.clear();
  }

  static const int NUM_BOXES_PER_BLOCK = 64 * 1024;
  BlockChain<Box, NUM_BOXES_PER_BLOCK> boxes_;
  BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> colors_;
  BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> picking_colors_;
  std::vector<std::unique_ptr<PickingUserData>> user_data_;
};

struct TriangleBuffer {
  void Reset() {
    triangles_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
    user_data_.clear();
  }

  static const int NUM_TRIANGLES_PER_BLOCK = 64 * 1024;
  BlockChain<Triangle, NUM_TRIANGLES_PER_BLOCK> triangles_;
  BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> colors_;
  BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> picking_colors_;
  std::vector<std::unique_ptr<PickingUserData>> user_data_;
};

class Batcher {
 public:
  explicit Batcher(BatcherId batcher_id,
                   PickingManager* picking_manager = nullptr)
      : batcher_id_(batcher_id), picking_manager_(picking_manager) {}

  Batcher() = delete;
  Batcher(const Batcher&) = delete;
  Batcher(Batcher&&) = delete;

  void AddLine(Vec2 from, Vec2 to, float z, const Color& color,
               std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                       std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color,
               std::weak_ptr<Pickable> pickable);
  void AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                       std::weak_ptr<Pickable> pickable);

  void AddBox(const Box& box, const std::array<Color, 4>& colors,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& box, const Color& color,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& box, const Color& color,
              std::weak_ptr<Pickable> pickable);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    std::unique_ptr<PickingUserData> user_data = nullptr);

  void AddTriangle(const Triangle& triangle, const Color& color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddTriangle(const Triangle& triangle, const Color& color,
                   std::weak_ptr<Pickable> pickable);

  virtual void Draw(bool picking = false) const;

  void Reset();

  [[nodiscard]] PickingManager* GetPickingManager() { return picking_manager_; }
  void SetPickingManager(PickingManager* picking_manager) {
    picking_manager_ = picking_manager;
  }

  [[nodiscard]] const PickingUserData* GetUserData(PickingId id) const;
  [[nodiscard]] PickingUserData* GetUserData(PickingId id);

  [[nodiscard]] TextBox* GetTextBox(PickingId id);

 protected:
  void DrawLineBuffer(bool picking) const;
  void DrawBoxBuffer(bool picking) const;
  void DrawTriangleBuffer(bool picking) const;

  void GetBoxGradientColors(const Color& color, std::array<Color, 4>* colors);

  void AddLine(Vec2 from, Vec2 to, float z, const Color& color,
               const Color& picking_color,
               std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& box, const std::array<Color, 4>& colors,
              const Color& picking_color,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddTriangle(const Triangle& triangle, const Color& color,
                   const Color& picking_color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);

  BatcherId batcher_id_;
  PickingManager* picking_manager_;
  LineBuffer line_buffer_;
  BoxBuffer box_buffer_;
  TriangleBuffer triangle_buffer_;
};

#endif
