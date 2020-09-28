// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include "OpenGl.h"
#include "Utils.h"

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(PickingType::kLine, user_data_.size(), batcher_id_);

  AddLine(from, to, z, color, picking_color, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      std::shared_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddLine(from, to, z, color, picking_color, nullptr);
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                              std::unique_ptr<PickingUserData> user_data) {
  AddLine(pos, pos + Vec2(0, size), z, color, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
                      std::unique_ptr<PickingUserData> user_data) {
  Line line;
  line.start_point = Vec3(from[0], from[1], z);
  line.end_point = Vec3(to[0], to[1], z);

  line_buffer_.lines_.push_back(line);
  line_buffer_.colors_.push_back_n(color, 2);
  line_buffer_.picking_colors_.push_back_n(picking_color, 2);
  user_data_.push_back(std::move(user_data));
}

void Batcher::AddBox(const Box& box, const std::array<Color, 4>& colors,
                     std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(PickingType::kBox, user_data_.size(), batcher_id_);
  AddBox(box, colors, picking_color, std::move(user_data));
}

void Batcher::AddBox(const Box& box, const Color& color,
                     std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  Fill(colors, color);
  AddBox(box, colors, std::move(user_data));
}

void Batcher::AddBox(const Box& box, const Color& color, std::shared_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);
  std::array<Color, 4> colors;
  Fill(colors, color);

  AddBox(box, colors, picking_color, nullptr);
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color) {
  AddShadedBox(pos, size, z, color, std::unique_ptr<PickingUserData>(),
               ShadingDirection::kLeftToRight);
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                           ShadingDirection shading_direction) {
  AddShadedBox(pos, size, z, color, std::unique_ptr<PickingUserData>(), shading_direction);
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                           std::unique_ptr<PickingUserData> user_data,
                           ShadingDirection shading_direction) {
  std::array<Color, 4> colors;
  GetBoxGradientColors(color, &colors, shading_direction);
  Box box(pos, size, z);
  AddBox(box, colors, std::move(user_data));
}

void Batcher::AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                           std::shared_ptr<Pickable> pickable, ShadingDirection shading_direction) {
  std::array<Color, 4> colors;
  GetBoxGradientColors(color, &colors, shading_direction);
  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);
  Box box(pos, size, z);
  AddBox(box, colors, picking_color, nullptr);
}

void Batcher::AddBox(const Box& box, const std::array<Color, 4>& colors, const Color& picking_color,
                     std::unique_ptr<PickingUserData> user_data) {
  box_buffer_.boxes_.push_back(box);
  box_buffer_.colors_.push_back(colors);
  box_buffer_.picking_colors_.push_back_n(picking_color, 4);
  user_data_.push_back(std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(PickingType::kTriangle, user_data_.size(), batcher_id_);

  AddTriangle(triangle, color, picking_color, std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          std::shared_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddTriangle(triangle, color, picking_color, nullptr);
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color, const Color& picking_color,
                          std::unique_ptr<PickingUserData> user_data) {
  triangle_buffer_.triangles_.push_back(triangle);
  triangle_buffer_.colors_.push_back_n(color, 3);
  triangle_buffer_.picking_colors_.push_back_n(picking_color, 3);
  user_data_.push_back(std::move(user_data));
}

void Batcher::AddCircle(Vec2 position, float radius, float z, Color color) {
  std::vector<Vec2> circle_points_scaled_by_radius;
  for (auto& point : circle_points) {
    circle_points_scaled_by_radius.emplace_back(radius * point);
  }

  Vec3 prev_point(position[0], position[1] - radius, z);
  Vec3 point_0 = Vec3(position[0], position[1], z);
  for (size_t i = 0; i < circle_points_scaled_by_radius.size(); ++i) {
    Vec3 new_point(position[0] + circle_points_scaled_by_radius[i][0],
                   position[1] - circle_points_scaled_by_radius[i][1], z);
    Vec3 point_1 = Vec3(prev_point[0], prev_point[1], z);
    Vec3 point_2 = Vec3(new_point[0], new_point[1], z);
    Triangle triangle(point_0, point_1, point_2);
    AddTriangle(triangle, color);
    prev_point = new_point;
  }
}

const PickingUserData* Batcher::GetUserData(PickingId id) const {
  CHECK(id.element_id >= 0);
  CHECK(id.batcher_id == batcher_id_);

  switch (id.type) {
    case PickingType::kInvalid:
      return nullptr;
    case PickingType::kBox:
    case PickingType::kTriangle:
    case PickingType::kLine:
      CHECK(id.element_id < user_data_.size());
      return user_data_[id.element_id].get();
    case PickingType::kPickable:
      return nullptr;
  }

  UNREACHABLE();
}

PickingUserData* Batcher::GetUserData(PickingId id) {
  return const_cast<PickingUserData*>(static_cast<const Batcher*>(this)->GetUserData(id));
}

const TextBox* Batcher::GetTextBox(PickingId id) {
  PickingUserData* data = GetUserData(id);

  if (data && data->text_box_) {
    return data->text_box_;
  }

  return nullptr;
}

void Batcher::GetBoxGradientColors(const Color& color, std::array<Color, 4>* colors,
                                   ShadingDirection shading_direction) {
  const float kGradientCoeff = 0.94f;
  Vec3 dark = Vec3(color[0], color[1], color[2]) * kGradientCoeff;
  Color dark_color = Color(static_cast<uint8_t>(dark[0]), static_cast<uint8_t>(dark[1]),
                           static_cast<uint8_t>(dark[2]), color[3]);

  switch (shading_direction) {
    case ShadingDirection::kLeftToRight:
      (*colors)[0] = dark_color;
      (*colors)[1] = dark_color;
      (*colors)[2] = color;
      (*colors)[3] = color;
      break;
    case ShadingDirection::kRightToLeft:
      (*colors)[0] = color;
      (*colors)[1] = color;
      (*colors)[2] = dark_color;
      (*colors)[3] = dark_color;
      break;
    case ShadingDirection::kTopToBottom:
      (*colors)[0] = dark_color;
      (*colors)[1] = color;
      (*colors)[2] = color;
      (*colors)[3] = dark_color;
      break;
    case ShadingDirection::kBottomToTop:
      (*colors)[0] = color;
      (*colors)[1] = dark_color;
      (*colors)[2] = dark_color;
      (*colors)[3] = color;
      break;
  }
}

void Batcher::ResetElements() {
  line_buffer_.Reset();
  box_buffer_.Reset();
  triangle_buffer_.Reset();
}

void Batcher::StartNewFrame() {
  ResetElements();
  user_data_.clear();
}

void Batcher::Draw(bool picking) const {
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
  if (picking) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
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
  const Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* box_block = box_buffer_.boxes_.root();
  const Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* color_block;

  color_block = !picking ? box_buffer_.colors_.root() : box_buffer_.picking_colors_.root();

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
  const Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* line_block = line_buffer_.lines_.root();
  const Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* color_block;

  color_block = !picking ? line_buffer_.colors_.root() : line_buffer_.picking_colors_.root();

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
  const Block<Triangle, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK>* triangle_block =
      triangle_buffer_.triangles_.root();
  const Block<Color, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK * 3>* color_block;

  color_block =
      !picking ? triangle_buffer_.colors_.root() : triangle_buffer_.picking_colors_.root();

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
