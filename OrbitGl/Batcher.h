//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once
#include <vector>

#include "BlockChain.h"
#include "Geometry.h"
#include "PickingManager.h"

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
  BlockChain<void*, NUM_LINES_PER_BLOCK> m_UserData;
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
  BlockChain<void*, NUM_BOXES_PER_BLOCK> m_UserData;
};

//-----------------------------------------------------------------------------
class Batcher {
 public:
  void AddLine(const Line& line, const Color* colors,
               PickingID::Type picking_type, void* user_data = nullptr);
  void AddLine(const Line& line, Color color, PickingID::Type picking_type,
               void* user_data = nullptr);
  void AddLine(Vec2 from, Vec2 to, float z, Color color,
               PickingID::Type picking_type, void* user_data = nullptr);
  void AddVerticalLine(Vec2 pos, float size, float z, Color color,
                       PickingID::Type picking_type, void* user_data = nullptr);

  void AddBox(const Box& a_Box, const Color* colors,
              PickingID::Type picking_type, void* user_data = nullptr);
  void AddBox(const Box& a_Box, Color color, PickingID::Type picking_type,
              void* user_data = nullptr);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, Color color,
                    PickingID::Type picking_type, void* user_data = nullptr);

  void GetBoxGradientColors(Color color, Color* colors);

  void Reset();

  TextBox* GetTextBox(PickingID a_ID);
  BoxBuffer& GetBoxBuffer() { return box_buffer_; }
  LineBuffer& GetLineBuffer() { return line_buffer_; }

 protected:
  LineBuffer line_buffer_;
  BoxBuffer box_buffer_;
};
