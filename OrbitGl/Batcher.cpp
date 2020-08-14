// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include "OpenGl.h"
#include "Utils.h"

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(
      PickingType::kLine, line_buffer_.lines_.size(), batcher_id_);

  AddLine(from, to, z, color, picking_color, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      std::weak_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color =
      picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddLine(from, to, z, color, picking_color, nullptr);
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                              std::unique_ptr<PickingUserData> user_data) {
  AddLine(pos, pos + Vec2(0, size), z, color, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      const Color& picking_color,
                      std::unique_ptr<PickingUserData> user_data) {
  Line line;
  line.m_Beg = Vec3(from[0], from[1], z);
  line.m_End = Vec3(to[0], to[1], z);

  line_buffer_.lines_.push_back(line);
  line_buffer_.colors_.push_back_n(color, 2);
  line_buffer_.picking_colors_.push_back_n(picking_color, 2);
  line_buffer_.user_data_.push_back(std::move(user_data));
}

void Batcher::AddBox(const Box& box, const std::array<Color, 4>& colors,
                     std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(
      PickingType::kBox, box_buffer_.boxes_.size(), batcher_id_);
  AddBox(box, colors, picking_color, std::move(user_data));
}

void Batcher::AddBox(const Box& box, const Color& color,
                     std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  Fill(colors, color);
  AddBox(box, colors, std::move(user_data));
}

void Batcher::AddBox(const Box& box, const Color& color,
                     std::weak_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color =
      picking_manager_->GetPickableColor(pickable, batcher_id_);
  std::array<Color, 4> colors;
  Fill(colors, color);

  AddBox(box, colors, picking_color, nullptr);
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                           std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  GetBoxGradientColors(color, &colors);
  Box box(pos, size, z);
  AddBox(box, colors, std::move(user_data));
}

void Batcher::AddBox(const Box& box, const std::array<Color, 4>& colors,
                     const Color& picking_color,
                     std::unique_ptr<PickingUserData> user_data) {
  box_buffer_.boxes_.push_back(box);
  box_buffer_.colors_.push_back(colors);
  box_buffer_.picking_colors_.push_back_n(picking_color, 4);
  box_buffer_.user_data_.push_back(std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(
      PickingType::kTriangle, triangle_buffer_.triangles_.size(), batcher_id_);

  AddTriangle(triangle, color, picking_color, std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          std::weak_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color =
      picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddTriangle(triangle, color, picking_color, nullptr);
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          const Color& picking_color,
                          std::unique_ptr<PickingUserData> user_data) {
  triangle_buffer_.triangles_.push_back(triangle);
  triangle_buffer_.colors_.push_back_n(color, 3);
  triangle_buffer_.picking_colors_.push_back_n(picking_color, 3);
  triangle_buffer_.user_data_.push_back(std::move(user_data));
}

PickingUserData* Batcher::GetUserData(PickingId id) {
  CHECK(id.element_id >= 0);
  CHECK(id.batcher_id == batcher_id_);

  switch (id.type) {
    case PickingType::kInvalid:
      return nullptr;
    case PickingType::kBox:
      CHECK(id.element_id < box_buffer_.user_data_.size());
      return box_buffer_.user_data_[id.element_id].get();
    case PickingType::kLine:
      CHECK(id.element_id < line_buffer_.user_data_.size());
      return line_buffer_.user_data_[id.element_id].get();
    case PickingType::kTriangle:
      CHECK(id.element_id < triangle_buffer_.user_data_.size());
      return triangle_buffer_.user_data_[id.element_id].get();
    case PickingType::kPickable:
      return nullptr;
  }

  UNREACHABLE();
}

TextBox* Batcher::GetTextBox(PickingId id) {
  PickingUserData* data = GetUserData(id);

  if (data && data->text_box_) {
    return data->text_box_;
  }

  return nullptr;
}

void Batcher::GetBoxGradientColors(const Color& color,
                                   std::array<Color, 4>* colors) {
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

void Batcher::Draw(bool picking) const {
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

void Batcher::DrawBoxBuffer(bool picking) const {
  const Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* box_block =
      box_buffer_.boxes_.root();
  const Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* color_block;

  color_block = !picking ? box_buffer_.colors_.root()
                         : box_buffer_.picking_colors_.root();

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

void Batcher::DrawLineBuffer(bool picking) const {
  const Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* line_block =
      line_buffer_.lines_.root();
  const Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* color_block;

  color_block = !picking ? line_buffer_.colors_.root()
                         : line_buffer_.picking_colors_.root();

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

void Batcher::DrawTriangleBuffer(bool picking) const {
  const Block<Triangle, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK>*
      triangle_buffer_.triangles_.root();
  const Block<Color, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK * 3>* color_block;

  color_block = !picking ? triangle_buffer_.colors_.root()
                         : triangle_buffer_.picking_colors_.root();

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
