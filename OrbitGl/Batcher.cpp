// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include "Core.h"
#include "OpenGl.h"

void Batcher::AddLine(const Line& line, const Color* colors,
                      PickingID::Type picking_type,
                      std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingID::GetColor(
      picking_type, line_buffer_.m_Lines.size(), batcher_id_);
  line_buffer_.m_Lines.push_back(line);
  line_buffer_.m_Colors.push_back(colors, 2);
  line_buffer_.m_PickingColors.push_back_n(picking_color, 2);
  line_buffer_.m_UserData.push_back(std::move(user_data));
}

void Batcher::AddLine(const Line& line, Color color,
                      PickingID::Type picking_type,
                      std::unique_ptr<PickingUserData> user_data) {
  Color colors[2];
  Fill(colors, color);
  AddLine(line, colors, picking_type, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, Color color,
                      PickingID::Type picking_type,
                      std::unique_ptr<PickingUserData> user_data) {
  Line line;
  Color colors[2];
  Fill(colors, color);
  line.m_Beg = Vec3(from[0], from[1], z);
  line.m_End = Vec3(to[0], to[1], z);
  AddLine(line, colors, picking_type, std::move(user_data));
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, Color color,
                              PickingID::Type picking_type,
                              std::unique_ptr<PickingUserData> user_data) {
  Line line;
  Color colors[2];
  Fill(colors, color);
  line.m_Beg = Vec3(pos[0], pos[1], z);
  line.m_End = Vec3(pos[0], pos[1] + size, z);
  AddLine(line, colors, picking_type, std::move(user_data));
}

void Batcher::AddBox(const Box& a_Box, const Color* colors,
                     PickingID::Type picking_type,
                     std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingID::GetColor(
      picking_type, box_buffer_.m_Boxes.size(), batcher_id_);
  box_buffer_.m_Boxes.push_back(a_Box);
  box_buffer_.m_Colors.push_back(colors, 4);
  box_buffer_.m_PickingColors.push_back_n(picking_color, 4);
  box_buffer_.m_UserData.push_back(std::move(user_data));
}

void Batcher::AddBox(const Box& a_Box, Color color,
                     PickingID::Type picking_type,
                     std::unique_ptr<PickingUserData> user_data) {
  Color colors[4];
  Fill(colors, color);
  AddBox(a_Box, colors, picking_type, std::move(user_data));
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, Color color,
                           PickingID::Type picking_type,
                           std::unique_ptr<PickingUserData> user_data) {
  Color colors[4];
  GetBoxGradientColors(color, colors);
  Box box(pos, size, z);
  AddBox(box, colors, picking_type, std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, Color color,
                          PickingID::Type picking_type,
                          std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingID::GetColor(
      picking_type, triangle_buffer_.triangles_.size(), batcher_id_);
  triangle_buffer_.triangles_.push_back(triangle);
  triangle_buffer_.colors_.push_back_n(color, 3);
  triangle_buffer_.picking_colors_.push_back_n(picking_color, 3);
  triangle_buffer_.user_data_.push_back(std::move(user_data));
}

void Batcher::AddTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color,
                          PickingID::Type picking_type,
                          std::unique_ptr<PickingUserData> user_data) {
  AddTriangle(Triangle(v0, v1, v2), color, picking_type, std::move(user_data));
}

PickingUserData* Batcher::GetUserData(PickingID a_ID) {
  CHECK(a_ID.m_Id >= 0);

  switch (a_ID.m_Type) {
    case PickingID::BOX:
      CHECK(a_ID.m_Id < box_buffer_.m_UserData.size());
      return box_buffer_.m_UserData[a_ID.m_Id].get();
    case PickingID::LINE:
      CHECK(a_ID.m_Id < line_buffer_.m_UserData.size());
      return line_buffer_.m_UserData[a_ID.m_Id].get();
    case PickingID::TRIANGLE:
      CHECK(a_ID.m_Id < triangle_buffer_.user_data_.size());
      return triangle_buffer_.user_data_[a_ID.m_Id].get();
  }

  return nullptr;
 ;
}

TextBox* Batcher::GetTextBox(PickingID a_ID) {
  PickingUserData* data = GetUserData(a_ID);

  if (data && data->m_TextBox) {
    return data->m_TextBox; 
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
  triangle_buffer_.Reset();
}

//----------------------------------------------------------------------------
void Batcher::Draw(bool picking) {
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);

  DrawBoxBuffer(picking);
  DrawLineBuffer(picking);
  DrawTriangleBuffer(picking);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopAttrib();
}

//----------------------------------------------------------------------------
void Batcher::DrawBoxBuffer(bool picking) {
  Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* box_block =
      GetBoxBuffer().m_Boxes.m_Root;
  Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* color_block;

  color_block = !picking ? GetBoxBuffer().m_Colors.m_Root
                         : GetBoxBuffer().m_PickingColors.m_Root;

  while (box_block) {
    if (int num_elems = box_block->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), box_block->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->m_Data);
      glDrawArrays(GL_QUADS, 0, num_elems * 4);
    }

    box_block = box_block->m_Next;
    color_block = color_block->m_Next;
  }
}

//----------------------------------------------------------------------------
void Batcher::DrawLineBuffer(bool picking) {
  Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* line_block =
      GetLineBuffer().m_Lines.m_Root;
  Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* color_block;

  color_block = !picking ? GetLineBuffer().m_Colors.m_Root
                         : GetLineBuffer().m_PickingColors.m_Root;

  while (line_block) {
    if (int num_elems = line_block->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), line_block->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->m_Data);
      glDrawArrays(GL_LINES, 0, num_elems * 2);
    }

    line_block = line_block->m_Next;
    color_block = color_block->m_Next;
  }
}

//----------------------------------------------------------------------------
void Batcher::DrawTriangleBuffer(bool picking) {
  Block<Triangle, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK>* triangle_block =
      GetTriangleBuffer().triangles_.m_Root;
  Block<Color, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK * 3>* color_block;

  color_block = !picking ? GetTriangleBuffer().colors_.m_Root
                         : GetTriangleBuffer().picking_colors_.m_Root;

  while (triangle_block) {
    if (int num_elems = triangle_block->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), triangle_block->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->m_Data);
      glDrawArrays(GL_TRIANGLES, 0, num_elems * 3);
    }

    triangle_block = triangle_block->m_Next;
    color_block = color_block->m_Next;
  }
}
