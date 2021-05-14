// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include <GteVector.h>
#include <GteVector2.h>
#include <glad/glad.h>
#include <math.h>
#include <stddef.h>

#include "CoreUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"

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

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                              std::shared_ptr<Pickable> pickable) {
  CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddLine(pos, pos + Vec2(0, size), z, color, picking_color, nullptr);
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
                      std::unique_ptr<PickingUserData> user_data) {
  Line line;
  line.start_point = Vec3(floorf(from[0]), floorf(from[1]), z);
  line.end_point = Vec3(floorf(to[0]), floorf(to[1]), z);
  auto& buffer = primitive_buffers_by_layer_[z];

  buffer.line_buffer.lines_.emplace_back(line);
  buffer.line_buffer.colors_.push_back_n(color, 2);
  buffer.line_buffer.picking_colors_.push_back_n(picking_color, 2);
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

static std::vector<Triangle> GetUnitArcTriangles(float angle_0, float angle_1, uint32_t num_sides) {
  std::vector<Triangle> triangles;
  const Vec3 origin(0, 0, 0);

  float increment_radians = std::fabs(angle_1 - angle_0) / static_cast<float>(num_sides);
  Vec3 last_point(cosf(angle_0), sinf(angle_0), 0);
  for (uint32_t i = 1; i <= num_sides; ++i) {
    float angle = angle_0 + static_cast<float>(i) * increment_radians;
    Vec3 current_point(cosf(angle), sinf(angle), 0);
    triangles.emplace_back(Triangle(origin, last_point, current_point));
    last_point = current_point;
  }

  return triangles;
}

static void AddRoundedCornerTriangles(Batcher* batcher, const std::vector<Triangle> unit_triangles,
                                      Vec2 pos, float radius, float z, const Color& color) {
  for (auto& unit_triangle : unit_triangles) {
    Triangle triangle = unit_triangle;
    triangle.vertices[1] *= radius;
    triangle.vertices[2] *= radius;

    for (size_t i = 0; i < 3; ++i) {
      triangle.vertices[i][0] += pos[0];
      triangle.vertices[i][1] += pos[1];
      triangle.vertices[i][2] = z;
    }

    batcher->AddTriangle(triangle, color);
  }
}

void Batcher::AddBottomLeftRoundedCorner(Vec2 pos, float radius, float z, const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(kPiFloat, 1.5f * kPiFloat, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void Batcher::AddTopLeftRoundedCorner(Vec2 pos, float radius, float z, const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(0.5f * kPiFloat, kPiFloat, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void Batcher::AddTopRightRoundedCorner(Vec2 pos, float radius, float z, const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(0, 0.5f * kPiFloat, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void Batcher::AddBottomRightRoundedCorner(Vec2 pos, float radius, float z, const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(-0.5f * kPiFloat, 0, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void Batcher::AddRoundedBox(Vec2 pos, Vec2 size, float z, float radius, const Color& color,
                            float margin) {
  const Vec2 extra_margin(margin, margin);
  pos -= extra_margin;
  size += 2.f * extra_margin;

  Box left_box(Vec2(pos[0], pos[1] + radius), Vec2(radius, size[1] - 2 * radius), z);
  Box middle_box(Vec2(pos[0] + radius, pos[1]), Vec2(size[0] - 2 * radius, size[1]), z);
  Box right_box(Vec2(pos[0] + size[0] - radius, pos[1] + radius),
                Vec2(radius, size[1] - 2 * radius), z);

  AddBox(left_box, color);
  AddBox(middle_box, color);
  AddBox(right_box, color);

  Vec2 bottom_left_pos(pos[0] + radius, pos[1] + radius);
  Vec2 top_left_pos(pos[0] + radius, pos[1] + size[1] - radius);
  Vec2 top_right_pos(pos[0] + size[0] - radius, pos[1] + size[1] - radius);
  Vec2 bottom_right_pos(pos[0] + size[0] - radius, pos[1] + radius);

  AddBottomLeftRoundedCorner(bottom_left_pos, radius, z, color);
  AddTopLeftRoundedCorner(top_left_pos, radius, z, color);
  AddTopRightRoundedCorner(top_right_pos, radius, z, color);
  AddBottomRightRoundedCorner(bottom_right_pos, radius, z, color);
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
  Box rounded_box = box;
  for (size_t v = 0; v < 4; ++v) {
    rounded_box.vertices[v][0] = floorf(rounded_box.vertices[v][0]);
    rounded_box.vertices[v][1] = floorf(rounded_box.vertices[v][1]);
  }
  float layer_z_value = rounded_box.vertices[0][2];
  auto& buffer = primitive_buffers_by_layer_[layer_z_value];
  buffer.box_buffer.boxes_.emplace_back(rounded_box);
  buffer.box_buffer.colors_.push_back(colors);
  buffer.box_buffer.picking_colors_.push_back_n(picking_color, 4);
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
  std::array<Color, 3> colors;
  colors.fill(color);
  AddTriangle(triangle, colors, picking_color, std::move(user_data));
}

// Draw a shaded trapezium with two sides parallel to the x-axis or y-axis.
void Batcher::AddShadedTrapezium(const Vec3& top_left, const Vec3& bottom_left,
                                 const Vec3& bottom_right, const Vec3& top_right,
                                 const Color& color, std::unique_ptr<PickingUserData> user_data,
                                 ShadingDirection shading_direction) {
  std::array<Color, 4> colors;  // top_left, bottom_left, bottom_right, top_right.
  GetBoxGradientColors(color, &colors, shading_direction);
  Color picking_color = PickingId::ToColor(PickingType::kTriangle, user_data_.size(), batcher_id_);
  Triangle triangle_1{top_left, bottom_left, top_right};
  std::array<Color, 3> colors_1{colors[0], colors[1], colors[2]};
  AddTriangle(triangle_1, colors_1, picking_color, std::make_unique<PickingUserData>(*user_data));
  Triangle triangle_2{bottom_left, bottom_right, top_right};
  std::array<Color, 3> colors_2{colors[1], colors[2], colors[3]};
  AddTriangle(triangle_2, colors_2, picking_color, std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const std::array<Color, 3>& colors,
                          const Color& picking_color, std::unique_ptr<PickingUserData> user_data) {
  Triangle rounded_tri = triangle;
  for (auto& vertice : rounded_tri.vertices) {
    vertice[0] = floorf(vertice[0]);
    vertice[1] = floorf(vertice[1]);
  }
  float layer_z_value = rounded_tri.vertices[0][2];
  auto& buffer = primitive_buffers_by_layer_[layer_z_value];
  buffer.triangle_buffer.triangles_.emplace_back(rounded_tri);
  buffer.triangle_buffer.colors_.push_back(colors);
  buffer.triangle_buffer.picking_colors_.push_back_n(picking_color, 3);
  user_data_.push_back(std::move(user_data));
}

void Batcher::AddCircle(Vec2 position, float radius, float z, Color color) {
  std::vector<Vec2> circle_points_scaled_by_radius;
  for (auto& point : circle_points) {
    circle_points_scaled_by_radius.emplace_back(radius * point);
  }

  position = Vec2(floorf(position[0]), floorf(position[1]));

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
    case PickingType::kCount:
      UNREACHABLE();
  }

  UNREACHABLE();
}

PickingUserData* Batcher::GetUserData(PickingId id) {
  return const_cast<PickingUserData*>(static_cast<const Batcher*>(this)->GetUserData(id));
}

const TextBox* Batcher::GetTextBox(PickingId id) const {
  const PickingUserData* data = GetUserData(id);

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
  for (auto& [unused_layer, buffer] : primitive_buffers_by_layer_) {
    buffer.Reset();
  }
}

void Batcher::StartNewFrame() {
  ResetElements();
  user_data_.clear();
}

std::vector<float> Batcher::GetLayers() const {
  std::vector<float> layers;
  for (auto& [layer, _] : primitive_buffers_by_layer_) {
    layers.push_back(layer);
  }
  return layers;
};

void Batcher::DrawLayer(float layer, bool picking) const {
  ORBIT_SCOPE_FUNCTION;
  if (!primitive_buffers_by_layer_.count(layer)) return;
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

  DrawBoxBuffer(layer, picking);
  DrawLineBuffer(layer, picking);
  DrawTriangleBuffer(layer, picking);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopAttrib();
}

void Batcher::Draw(bool picking) const {
  for (auto& [layer, unused_buffer] : primitive_buffers_by_layer_) {
    DrawLayer(layer, picking);
  }
}

void Batcher::DrawBoxBuffer(float layer, bool picking) const {
  auto& box_buffer = primitive_buffers_by_layer_.at(layer).box_buffer;
  const Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* box_block = box_buffer.boxes_.root();
  const Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* color_block;

  color_block = !picking ? box_buffer.colors_.root() : box_buffer.picking_colors_.root();

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

void Batcher::DrawLineBuffer(float layer, bool picking) const {
  auto& line_buffer = primitive_buffers_by_layer_.at(layer).line_buffer;
  const Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* line_block = line_buffer.lines_.root();
  const Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* color_block;

  color_block = !picking ? line_buffer.colors_.root() : line_buffer.picking_colors_.root();

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

void Batcher::DrawTriangleBuffer(float layer, bool picking) const {
  auto& triangle_buffer = primitive_buffers_by_layer_.at(layer).triangle_buffer;
  const Block<Triangle, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK>* triangle_block =
      triangle_buffer.triangles_.root();
  const Block<Color, TriangleBuffer::NUM_TRIANGLES_PER_BLOCK * 3>* color_block;

  color_block = !picking ? triangle_buffer.colors_.root() : triangle_buffer.picking_colors_.root();

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
