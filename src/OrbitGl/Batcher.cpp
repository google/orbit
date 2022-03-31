// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Batcher.h"

#include <OrbitBase/Logging.h>
#include <math.h>
#include <stddef.h>

#include "Geometry.h"

namespace orbit_gl {

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(PickingType::kLine, user_data_.size(), batcher_id_);

  AddLineInternal(from, to, z, color, picking_color, std::move(user_data));
}

void Batcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                      std::shared_ptr<Pickable> pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddLineInternal(from, to, z, color, picking_color, nullptr);
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                              std::unique_ptr<PickingUserData> user_data) {
  AddLine(pos, pos + Vec2(0, size), z, color, std::move(user_data));
}

void Batcher::AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                              std::shared_ptr<Pickable> pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddLineInternal(pos, pos + Vec2(0, size), z, color, picking_color, nullptr);
}

void Batcher::AddBox(const Tetragon& box, const std::array<Color, 4>& colors,
                     std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(PickingType::kBox, user_data_.size(), batcher_id_);
  AddBoxInternal(box, colors, picking_color, std::move(user_data));
}

void Batcher::AddBox(const Tetragon& box, const Color& color,
                     std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  colors.fill(color);
  AddBox(box, colors, std::move(user_data));
}

void Batcher::AddBox(const Tetragon& box, const Color& color, std::shared_ptr<Pickable> pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);
  std::array<Color, 4> colors;
  colors.fill(color);

  AddBoxInternal(box, colors, picking_color, nullptr);
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
  Tetragon box = MakeBox(pos, size, z);
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

  Tetragon left_box = MakeBox(Vec2(pos[0], pos[1] + radius), Vec2(radius, size[1] - 2 * radius), z);
  Tetragon middle_box =
      MakeBox(Vec2(pos[0] + radius, pos[1]), Vec2(size[0] - 2 * radius, size[1]), z);
  Tetragon right_box = MakeBox(Vec2(pos[0] + size[0] - radius, pos[1] + radius),
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
  Tetragon box = MakeBox(pos, size, z);
  AddBoxInternal(box, colors, picking_color, nullptr);
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          std::unique_ptr<PickingUserData> user_data) {
  Color picking_color = PickingId::ToColor(PickingType::kTriangle, user_data_.size(), batcher_id_);

  AddTriangle(triangle, color, picking_color, std::move(user_data));
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color,
                          std::shared_ptr<Pickable> pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, batcher_id_);

  AddTriangle(triangle, color, picking_color, nullptr);
}

void Batcher::AddTriangle(const Triangle& triangle, const Color& color, const Color& picking_color,
                          std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 3> colors;
  colors.fill(color);
  AddTriangleInternal(triangle, colors, picking_color, std::move(user_data));
}

// Draw a shaded trapezium with two sides parallel to the x-axis or y-axis.
void Batcher::AddShadedTrapezium(const Tetragon& trapezium, const Color& color,
                                 std::unique_ptr<PickingUserData> user_data,
                                 ShadingDirection shading_direction) {
  std::array<Color, 4> colors;  // top_left, bottom_left, bottom_right, top_right.
  GetBoxGradientColors(color, &colors, shading_direction);
  Color picking_color = PickingId::ToColor(PickingType::kTriangle, user_data_.size(), batcher_id_);
  Triangle triangle_1{trapezium.TopLeft(), trapezium.BottomLeft(), trapezium.TopRight()};
  std::array<Color, 3> colors_1{colors[0], colors[1], colors[2]};
  AddTriangleInternal(triangle_1, colors_1, picking_color,
                      std::make_unique<PickingUserData>(*user_data));
  Triangle triangle_2{trapezium.BottomLeft(), trapezium.BottomRight(), trapezium.TopRight()};
  std::array<Color, 3> colors_2{colors[1], colors[2], colors[3]};
  AddTriangleInternal(triangle_2, colors_2, picking_color, std::move(user_data));
}

void Batcher::AddCircle(const Vec2& position, float radius, float z, Color color) {
  std::vector<Vec2> circle_points_scaled_by_radius;
  for (auto& point : circle_points) {
    circle_points_scaled_by_radius.emplace_back(radius * point);
  }

  Vec3 final_position = translations_.TranslateAndFloorVertex(Vec3(position[0], position[1], z));

  Vec3 prev_point(final_position[0], final_position[1] - radius, z);
  Vec3 point_0 = final_position;
  for (size_t i = 0; i < circle_points_scaled_by_radius.size(); ++i) {
    Vec3 new_point(final_position[0] + circle_points_scaled_by_radius[i][0],
                   final_position[1] - circle_points_scaled_by_radius[i][1], z);
    Vec3 point_1 = Vec3(prev_point[0], prev_point[1], z);
    Vec3 point_2 = Vec3(new_point[0], new_point[1], z);
    Triangle triangle(point_0, point_1, point_2);
    AddTriangle(triangle, color);
    prev_point = new_point;
  }
}

const PickingUserData* Batcher::GetUserData(PickingId id) const {
  ORBIT_CHECK(id.element_id >= 0);
  ORBIT_CHECK(id.batcher_id == batcher_id_);

  switch (id.type) {
    case PickingType::kInvalid:
      return nullptr;
    case PickingType::kBox:
    case PickingType::kTriangle:
    case PickingType::kLine:
      ORBIT_CHECK(id.element_id < user_data_.size());
      return user_data_[id.element_id].get();
    case PickingType::kPickable:
      return nullptr;
    case PickingType::kCount:
      ORBIT_UNREACHABLE();
  }

  ORBIT_UNREACHABLE();
}

PickingUserData* Batcher::GetUserData(PickingId id) {
  return const_cast<PickingUserData*>(static_cast<const Batcher*>(this)->GetUserData(id));
}

const orbit_client_protos::TimerInfo* Batcher::GetTimerInfo(PickingId id) const {
  const PickingUserData* data = GetUserData(id);

  if (data && data->timer_info_) {
    return data->timer_info_;
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

void Batcher::StartNewFrame() {
  ORBIT_CHECK(translations_.IsEmpty());
  ResetElements();
}

void Batcher::Draw(bool picking) const {
  for (float layer : GetLayers()) {
    DrawLayer(layer, picking);
  }
}

}  // namespace orbit_gl