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
  void AddLine(const Line& a_Line, Color* a_Colors, PickingID::Type a_Type,
               void* a_UserData = nullptr) {
    Color pickCol = PickingID::GetColor(a_Type, m_LineBuffer.m_Lines.size());
    m_LineBuffer.m_Lines.push_back(a_Line);
    m_LineBuffer.m_Colors.push_back(a_Colors, 2);
    m_LineBuffer.m_PickingColors.push_back_n(pickCol, 2);
    m_LineBuffer.m_UserData.push_back(a_UserData);
  }

  void AddBox(const Box& a_Box, Color* a_Colors, PickingID::Type a_Type,
              void* a_UserData = nullptr) {
    Color pickCol = PickingID::GetColor(a_Type, m_BoxBuffer.m_Boxes.size());
    m_BoxBuffer.m_Boxes.push_back(a_Box);
    m_BoxBuffer.m_Colors.push_back(a_Colors, 4);
    m_BoxBuffer.m_PickingColors.push_back_n(pickCol, 4);
    m_BoxBuffer.m_UserData.push_back(a_UserData);
  }

  void Reset() {
    m_LineBuffer.Reset();
    m_BoxBuffer.Reset();
  }

  TextBox* GetTextBox(PickingID a_ID);

  BoxBuffer& GetBoxBuffer() { return m_BoxBuffer; }
  LineBuffer& GetLineBuffer() { return m_LineBuffer; }

 protected:
  LineBuffer m_LineBuffer;
  BoxBuffer m_BoxBuffer;
};
