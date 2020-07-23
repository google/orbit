// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include <vector>

#include "BlockChain.h"
#include "Geometry.h"
#include "PickingManager.h"

//-----------------------------------------------------------------------------
using TooltipCallback = std::function<std::string(PickingID)>;

struct PickingUserData {
  TextBox* m_TextBox;
  TooltipCallback m_GenerateTooltip;

  PickingUserData(
    TextBox* text_box = nullptr,
    TooltipCallback generate_tooltip = nullptr)
    : m_TextBox(text_box), m_GenerateTooltip(generate_tooltip) {}
};

//-----------------------------------------------------------------------------
struct LineBuffer {
  void Reset() {
    m_Lines.Reset();
    m_Colors.Reset();
    m_PickingColors.Reset();
    m_UserData.Reset();
  }

  static const int NUM_LINES_PER_BLOCK = 64 * 1024;
  BlockChain<Line, NUM_LINES_PER_BLOCK> m_Lines;
  BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> m_Colors;
  BlockChain<Color, 2 * NUM_LINES_PER_BLOCK> m_PickingColors;
  BlockChain<std::shared_ptr<PickingUserData>, NUM_LINES_PER_BLOCK> m_UserData;
};

//-----------------------------------------------------------------------------
struct BoxBuffer {
  void Reset() {
    m_Boxes.Reset();
    m_Colors.Reset();
    m_PickingColors.Reset();
    m_UserData.Reset();
  }

  static const int NUM_BOXES_PER_BLOCK = 64 * 1024;
  BlockChain<Box, NUM_BOXES_PER_BLOCK> m_Boxes;
  BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> m_Colors;
  BlockChain<Color, 4 * NUM_BOXES_PER_BLOCK> m_PickingColors;
  BlockChain<std::shared_ptr<PickingUserData>, NUM_BOXES_PER_BLOCK> m_UserData;
};

//-----------------------------------------------------------------------------
struct TriangleBuffer {
  void Reset() {
    triangles_.Reset();
    colors_.Reset();
    picking_colors_.Reset();
    user_data_.Reset();
  }

  static const int NUM_TRIANGLES_PER_BLOCK = 64 * 1024;
  BlockChain<Triangle, NUM_TRIANGLES_PER_BLOCK> triangles_;
  BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> colors_;
  BlockChain<Color, 3 * NUM_TRIANGLES_PER_BLOCK> picking_colors_;
  BlockChain<std::shared_ptr<PickingUserData>, NUM_TRIANGLES_PER_BLOCK>
    user_data_;
};

//-----------------------------------------------------------------------------
class Batcher {
 public:
  explicit Batcher(PickingID::BatcherId batcher_id) : batcher_id_(batcher_id) {}
  Batcher() : batcher_id_(PickingID::BatcherId::TIME_GRAPH) {}

  void AddLine(const Line& line, const Color* colors,
               PickingID::Type picking_type, 
               std::shared_ptr<PickingUserData> user_data = nullptr);
  void AddLine(const Line& line, Color color, PickingID::Type picking_type,
               std::shared_ptr<PickingUserData> user_data = nullptr);
  void AddLine(Vec2 from, Vec2 to, float z, Color color,
               PickingID::Type picking_type,
               std::shared_ptr<PickingUserData> user_data = nullptr);
  void AddVerticalLine(Vec2 pos, float size, float z, Color color,
                       PickingID::Type picking_type,
                       std::shared_ptr<PickingUserData> user_data = nullptr);

  void AddBox(const Box& a_Box, const Color* colors,
              PickingID::Type picking_type,
              std::shared_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Box& a_Box, Color color, PickingID::Type picking_type,
              std::shared_ptr<PickingUserData> user_data = nullptr);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, Color color,
                    PickingID::Type picking_type,
                    std::shared_ptr<PickingUserData> user_data = nullptr);

  void AddTriangle(const Triangle& triangle, Color color,
                   PickingID::Type picking_type,
                   std::shared_ptr<PickingUserData> user_data = nullptr);
  void AddTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color,
                   PickingID::Type picking_type,
                   std::shared_ptr<PickingUserData> user_data = nullptr);

  void GetBoxGradientColors(Color color, Color* colors);

  void Draw(bool picking = false);

  void Reset();

  std::shared_ptr<PickingUserData> GetUserData(PickingID a_ID);
  TextBox* GetTextBox(PickingID a_ID);
  BoxBuffer& GetBoxBuffer() { return box_buffer_; }
  LineBuffer& GetLineBuffer() { return line_buffer_; }
  TriangleBuffer& GetTriangleBuffer() { return triangle_buffer_; }

 protected:
  void DrawLineBuffer(bool picking);
  void DrawBoxBuffer(bool picking);
  void DrawTriangleBuffer(bool picking);
  LineBuffer line_buffer_;
  BoxBuffer box_buffer_;
  TriangleBuffer triangle_buffer_;
  PickingID::BatcherId batcher_id_;
};