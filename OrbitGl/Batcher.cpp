// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include "Core.h"
#include "GlCanvas.h"

void Batcher::AddLine(const Line& line, const Color* colors,
                      PickingID::Type picking_type, void* user_data) {
  Color pickCol =
      PickingID::GetColor(picking_type, line_buffer_.m_Lines.size());
  line_buffer_.m_Lines.push_back(line);
  line_buffer_.m_Colors.push_back(colors, 2);
  line_buffer_.m_PickingColors.push_back_n(pickCol, 2);
  line_buffer_.m_UserData.push_back(user_data);
}

void Batcher::AddLine(const Line& line, Color color,
                      PickingID::Type picking_type, void* user_data) {
  Color colors[2];
  Fill(colors, color);
  AddLine(line, colors, picking_type, user_data);
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, Color color,
                      PickingID::Type picking_type, void* user_data) {
  Line line;
  Color colors[2];
  Fill(colors, color);
  line.m_Beg = Vec3(from[0], from[1], z);
  line.m_End = Vec3(to[0], to[1], z);
  AddLine(line, colors, picking_type, user_data);
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, Color color,
                              PickingID::Type picking_type, void* user_data) {
  Line line;
  Color colors[2];
  Fill(colors, color);
  line.m_Beg = Vec3(pos[0], pos[1], z);
  line.m_End = Vec3(pos[0], pos[1] + size, z);
  AddLine(line, colors, picking_type, user_data);
}

void Batcher::AddBox(const Box& a_Box, const Color* colors,
                     PickingID::Type picking_type, void* user_data) {
  Color pickCol = PickingID::GetColor(picking_type, box_buffer_.m_Boxes.size());
  box_buffer_.m_Boxes.push_back(a_Box);
  box_buffer_.m_Colors.push_back(colors, 4);
  box_buffer_.m_PickingColors.push_back_n(pickCol, 4);
  box_buffer_.m_UserData.push_back(user_data);
}

void Batcher::AddBox(const Box& a_Box, Color color,
                     PickingID::Type picking_type, void* user_data) {
  Color colors[4];
  Fill(colors, color);
  AddBox(a_Box, colors, picking_type, user_data);
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, Color color,
                           PickingID::Type picking_type, void* user_data) {
  Color colors[4];
  GetBoxGradientColors(color, colors);
  Box box(pos, size, z);
  AddBox(box, colors, picking_type, user_data);
}

TextBox* Batcher::GetTextBox(PickingID a_ID) {
  if (a_ID.m_Type == PickingID::BOX) {
    if (void** textBoxPtr = box_buffer_.m_UserData.SlowAt(a_ID.m_Id)) {
      return static_cast<TextBox*>(*textBoxPtr);
    }
  } else if (a_ID.m_Type == PickingID::LINE) {
    if (void** textBoxPtr = line_buffer_.m_UserData.SlowAt(a_ID.m_Id)) {
      return static_cast<TextBox*>(*textBoxPtr);
    }
  }

  return nullptr;
}

void Batcher::GetBoxGradientColors(Color color, Color* colors) {
  const float kGradientCoeff = 0.94f;
  Vec3 dark = Vec3(color[0], color[1], color[2]) * kGradientCoeff;
  colors[0] =
      Color(static_cast<uint8_t>(dark[0]), static_cast<uint8_t>(dark[1]),
            static_cast<uint8_t>(dark[2]), color[3]);
  colors[1] = colors[0];
  colors[2] = color;
  colors[3] = color;
}

void Batcher::Reset() {
  line_buffer_.Reset();
  box_buffer_.Reset();
}

//----------------------------------------------------------------------------
void Batcher::Draw(bool a_Picking) {
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);

  DrawBoxBuffer(a_Picking);
  DrawLineBuffer(a_Picking);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopAttrib();
}

//----------------------------------------------------------------------------
void Batcher::DrawBoxBuffer(bool a_Picking) {
  Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* boxBlock =
      GetBoxBuffer().m_Boxes.m_Root;
  Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* colorBlock;

  colorBlock = !a_Picking ? GetBoxBuffer().m_Colors.m_Root
                          : GetBoxBuffer().m_PickingColors.m_Root;

  while (boxBlock) {
    if (int numElems = boxBlock->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), boxBlock->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), colorBlock->m_Data);
      glDrawArrays(GL_QUADS, 0, numElems * 4);
    }

    boxBlock = boxBlock->m_Next;
    colorBlock = colorBlock->m_Next;
  }
}

//----------------------------------------------------------------------------
void Batcher::DrawLineBuffer(bool a_Picking) {
  Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* lineBlock =
      GetLineBuffer().m_Lines.m_Root;
  Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* colorBlock;

  colorBlock = !a_Picking ? GetLineBuffer().m_Colors.m_Root
                          : GetLineBuffer().m_PickingColors.m_Root;

  while (lineBlock) {
    if (int numElems = lineBlock->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), lineBlock->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), colorBlock->m_Data);
      glDrawArrays(GL_LINES, 0, numElems * 2);
    }

    lineBlock = lineBlock->m_Next;
    colorBlock = colorBlock->m_Next;
  }
}
