// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "Batcher.h"

#include "Core.h"

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
