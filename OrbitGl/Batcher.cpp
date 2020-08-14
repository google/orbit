// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include "OpenGl.h"
#include "Utils.h"

void Batcher::AddLine(const Line& line, const std::array<Color, 2>& colors,
                      PickingType picking_type,
                      std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::GetColor(
      picking_type, line_buffer_.lines_.size(), batcher_id_);
  line_buffer_.lines_.push_back(line);
  line_buffer_.colors_.push_back(colors);
  line_buffer_.picking_colors_.push_back_n(picking_color, 2);
  line_buffer_.user_data_.push_back(std::move(user_data));
}

void Batcher::AddLine(const Line& line, Color color, PickingType picking_type,
                      std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 2> colors;
  Fill(colors, color);
  AddLine(line, colors, picking_type, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, Color color,
                      PickingType picking_type,
                      std::unique_ptr<PickingUserData> user_data) {
  Line line;
  std::array<Color, 2> colors;
  Fill(colors, color);
  line.m_Beg = Vec3(from[0], from[1], z);
  line.m_End = Vec3(to[0], to[1], z);
  AddLine(line, colors, picking_type, std::move(user_data));
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, Color color,
                              PickingType picking_type,
                              std::unique_ptr<PickingUserData> user_data) {
  Line line;
  std::array<Color, 2> colors;
  Fill(colors, color);
  line.m_Beg = Vec3(pos[0], pos[1], z);
  line.m_End = Vec3(pos[0], pos[1] + size, z);
  AddLine(line, colors, picking_type, std::move(user_data));
}

void Batcher::AddBox(const Box& box, const std::array<Color, 4>& colors,
                     PickingType picking_type,
                     std::unique_ptr<PickingUserData> user_data) {
  Color picking_color =
      PickingId::GetColor(picking_type, box_buffer_.boxes_.size(), batcher_id_);
  box_buffer_.boxes_.push_back(box);
  box_buffer_.colors_.push_back(colors);
  box_buffer_.picking_colors_.push_back_n(picking_color, 4);
  box_buffer_.user_data_.push_back(std::move(user_data));
}

void Batcher::AddBox(const Box& box, Color color, PickingType picking_type,
                     std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  Fill(colors, color);
  AddBox(box, colors, picking_type, std::move(user_data));
}

void Batcher::AddShadedBox(const Vec2& pos, const Vec2& size, float z,
                           const Color& color, PickingType picking_type,
                           std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  GetBoxGradientColors(color, &colors);
  Box box(pos, size, z);
  AddBox(box, colors, picking_type, std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          PickingType picking_type,
                          std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::GetColor(
      picking_type, triangle_buffer_.triangles_.size(), batcher_id_);
  triangle_buffer_.triangles_.push_back(triangle);
  triangle_buffer_.colors_.push_back_n(color, 3);
  triangle_buffer_.picking_colors_.push_back_n(picking_color, 3);
  triangle_buffer_.user_data_.push_back(std::move(user_data));
}

void Batcher::AddTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2,
                          const Color& color, PickingType picking_type,
                          std::unique_ptr<PickingUserData> user_data) {
  AddTriangle(Triangle(v0, v1, v2), color, picking_type, std::move(user_data));
}

PickingUserData* Batcher::GetUserData(PickingId id) {
  CHECK(id.id >= 0);

  switch (id.type) {
    case PickingType::kInvalid:
      return nullptr;
    case PickingType::kBox:
      CHECK(id.id < box_buffer_.user_data_.size());
      return box_buffer_.user_data_[id.id].get();
    case PickingType::kLine:
      CHECK(id.id < line_buffer_.user_data_.size());
      return line_buffer_.user_data_[id.id].get();
    case PickingType::kTriangle:
      CHECK(id.id < triangle_buffer_.user_data_.size());
      return triangle_buffer_.user_data_[id.id].get();
    case PickingType::kPickable:
      return nullptr;
  }
  UNREACHABLE();
}

TextBox* Batcher::GetTextBox(PickingId id) {
  PickingUserData* data = GetUserData(id);

  if (data != nullptr && data->text_box_ != nullptr) {
    return data->text_box_;
  }

  return nullptr;
}

void Batcher::GetBoxGradientColors(Color color, std::array<Color, 4>* colors) {
  const float kGradientCoeff = 0.94f;
  Vec3 dark = Vec3(color[0], color[1], color[2]) * kGradientCoeff;
  (*colors)[0] =
      Color(static_cast<uint8_t>(dark[0]), static_cast<uint8_t>(dark[1]),
            static_cast<uint8_t>(dark[2]), color[3]);
  (*colors)[1] = (*colors)[0];
  (*colors)[2] = color;
  (*colors)[3] = color;
}

void Batcher::Reset() {
  line_buffer_.Reset();
  box_buffer_.Reset();
  triangle_buffer_.Reset();
}

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

void Batcher::DrawBoxBuffer(bool picking) {
  const Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* box_block =
      GetBoxBuffer().boxes_.root();
  const Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* color_block;

  color_block = !picking ? GetBoxBuffer().colors_.root()
                         : GetBoxBuffer().picking_colors_.root();

  while (box_block != nullptr) {
    if (auto num_elems = box_block->size()) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), box_block->data());
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->data());
      glDrawArrays(GL_QUADS, 0, num_elems * 4);
    }

    box_block = box_block->next();
    color_block = color_block->next();
  }
}

void Batcher::DrawLineBuffer(bool picking) {
  const Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* line_block =
      GetLineBuffer().lines_.root();
  const Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* color_block;

  color_block = !picking ? GetLineBuffer().colors_.root()
                         : GetLineBuffer().picking_colors_.root();

  while (line_block != nullptr) {
    if (auto num_elems = line_block->size()) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), line_block->data());
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->data());
      glDrawArrays(GL_LINES, 0, num_elems * 2);
    }

    line_block = line_block->next();
    color_block = color_block->next();
  }
}

void Batcher::DrawTriangleBuffer(bool picking) {
  const Block<Triangle, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK>*
      triangle_block = GetTriangleBuffer().triangles_.root();
  const Block<Color, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK * 3>* color_block;

  color_block = !picking ? GetTriangleBuffer().colors_.root()
                         : GetTriangleBuffer().picking_colors_.root();

  while (triangle_block != nullptr) {
    if (int num_elems = triangle_block->size()) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), triangle_block->data());
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->data());
      glDrawArrays(GL_TRIANGLES, 0, num_elems * 3);
    }

    triangle_block = triangle_block->next();
    color_block = color_block->next();
  }
}
