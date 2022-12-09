// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/PrimitiveAssembler.h"

#include <GteVector.h>
#include <GteVector2.h>
#include <GteVector4.h>
#include <OrbitBase/Logging.h>
#include <stddef.h>

#include <array>
#include <cmath>
#include <utility>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"

namespace orbit_gl {

void PrimitiveAssembler::AddLine(const Vec2& from, const Vec2& to, float z, const Color& color,
                                 std::unique_ptr<PickingUserData> user_data) {
  Color picking_color =
      PickingId::ToColor(PickingType::kLine, batcher_->GetNumElements(), GetBatcherId());

  batcher_->AddLine(from, to, z, color, picking_color, std::move(user_data));
}

void PrimitiveAssembler::AddLine(const Vec2& from, const Vec2& to, float z, const Color& color,
                                 const std::shared_ptr<Pickable>& pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, GetBatcherId());

  batcher_->AddLine(from, to, z, color, picking_color, nullptr);
}

void PrimitiveAssembler::AddVerticalLine(const Vec2& pos, float size, float z, const Color& color,
                                         std::unique_ptr<PickingUserData> user_data) {
  AddLine(pos, pos + Vec2(0, size), z, color, std::move(user_data));
}

void PrimitiveAssembler::AddVerticalLine(const Vec2& pos, float size, float z, const Color& color,
                                         const std::shared_ptr<Pickable>& pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, GetBatcherId());

  batcher_->AddLine(pos, pos + Vec2(0, size), z, color, picking_color, nullptr);
}

void PrimitiveAssembler::AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
                                std::unique_ptr<PickingUserData> user_data) {
  Color picking_color =
      PickingId::ToColor(PickingType::kBox, batcher_->GetNumElements(), GetBatcherId());
  batcher_->AddBox(box, z, colors, picking_color, std::move(user_data));
}

void PrimitiveAssembler::AddBox(const Quad& box, float z, const Color& color,
                                std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 4> colors;
  colors.fill(color);
  AddBox(box, z, colors, std::move(user_data));
}

void PrimitiveAssembler::AddBox(const Quad& box, float z, const Color& color,
                                const std::shared_ptr<Pickable>& pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, GetBatcherId());
  std::array<Color, 4> colors;
  colors.fill(color);

  batcher_->AddBox(box, z, colors, picking_color, nullptr);
}

void PrimitiveAssembler::AddShadedBox(const Vec2& pos, const Vec2& size, float z,
                                      const Color& color) {
  AddShadedBox(pos, size, z, color, std::unique_ptr<PickingUserData>(),
               ShadingDirection::kLeftToRight);
}

void PrimitiveAssembler::AddShadedBox(const Vec2& pos, const Vec2& size, float z,
                                      const Color& color, ShadingDirection shading_direction) {
  AddShadedBox(pos, size, z, color, std::unique_ptr<PickingUserData>(), shading_direction);
}

void PrimitiveAssembler::AddShadedBox(const Vec2& pos, const Vec2& size, float z,
                                      const Color& color,
                                      std::unique_ptr<PickingUserData> user_data,
                                      ShadingDirection shading_direction) {
  std::array<Color, 4> colors;
  GetBoxGradientColors(color, &colors, shading_direction);
  Quad box = MakeBox(pos, size);
  AddBox(box, z, colors, std::move(user_data));
}

static std::vector<Triangle> GetUnitArcTriangles(float angle_0, float angle_1, uint32_t num_sides) {
  std::vector<Triangle> triangles;
  const Vec2 origin(0, 0);

  float increment_radians = std::fabs(angle_1 - angle_0) / static_cast<float>(num_sides);
  Vec2 last_point(std::cos(angle_0), std::sin(angle_0));
  for (uint32_t i = 1; i <= num_sides; ++i) {
    float angle = angle_0 + static_cast<float>(i) * increment_radians;
    Vec2 current_point(std::cos(angle), std::sin(angle));
    triangles.emplace_back(Triangle(origin, last_point, current_point));
    last_point = current_point;
  }

  return triangles;
}

static void AddRoundedCornerTriangles(PrimitiveAssembler* batcher,
                                      const std::vector<Triangle>& unit_triangles, Vec2 pos,
                                      float radius, float z, const Color& color) {
  for (const auto& unit_triangle : unit_triangles) {
    Triangle triangle = unit_triangle;
    triangle.vertices[1] *= radius;
    triangle.vertices[2] *= radius;

    for (size_t i = 0; i < 3; ++i) {
      triangle.vertices[i][0] += pos[0];
      triangle.vertices[i][1] += pos[1];
    }

    batcher->AddTriangle(triangle, z, color);
  }
}

void PrimitiveAssembler::AddBottomLeftRoundedCorner(const Vec2& pos, float radius, float z,
                                                    const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(kPiFloat, 1.5f * kPiFloat, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void PrimitiveAssembler::AddTopLeftRoundedCorner(const Vec2& pos, float radius, float z,
                                                 const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(0.5f * kPiFloat, kPiFloat, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void PrimitiveAssembler::AddTopRightRoundedCorner(const Vec2& pos, float radius, float z,
                                                  const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(0, 0.5f * kPiFloat, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void PrimitiveAssembler::AddBottomRightRoundedCorner(const Vec2& pos, float radius, float z,
                                                     const Color& color) {
  static auto unit_triangles = GetUnitArcTriangles(-0.5f * kPiFloat, 0, kNumArcSides);
  AddRoundedCornerTriangles(this, unit_triangles, pos, radius, z, color);
}

void PrimitiveAssembler::AddRoundedBox(Vec2 pos, Vec2 size, float z, float radius,
                                       const Color& color, float margin) {
  const Vec2 extra_margin(margin, margin);
  pos -= extra_margin;
  size += 2.f * extra_margin;

  Quad left_box = MakeBox(Vec2(pos[0], pos[1] + radius), Vec2(radius, size[1] - 2 * radius));
  Quad middle_box = MakeBox(Vec2(pos[0] + radius, pos[1]), Vec2(size[0] - 2 * radius, size[1]));
  Quad right_box =
      MakeBox(Vec2(pos[0] + size[0] - radius, pos[1] + radius), Vec2(radius, size[1] - 2 * radius));

  AddBox(left_box, z, color);
  AddBox(middle_box, z, color);
  AddBox(right_box, z, color);

  Vec2 bottom_left_pos(pos[0] + radius, pos[1] + radius);
  Vec2 top_left_pos(pos[0] + radius, pos[1] + size[1] - radius);
  Vec2 top_right_pos(pos[0] + size[0] - radius, pos[1] + size[1] - radius);
  Vec2 bottom_right_pos(pos[0] + size[0] - radius, pos[1] + radius);

  AddBottomLeftRoundedCorner(bottom_left_pos, radius, z, color);
  AddTopLeftRoundedCorner(top_left_pos, radius, z, color);
  AddTopRightRoundedCorner(top_right_pos, radius, z, color);
  AddBottomRightRoundedCorner(bottom_right_pos, radius, z, color);
}

void PrimitiveAssembler::AddShadedBox(const Vec2& pos, const Vec2& size, float z,
                                      const Color& color, const std::shared_ptr<Pickable>& pickable,
                                      ShadingDirection shading_direction) {
  std::array<Color, 4> colors;
  GetBoxGradientColors(color, &colors, shading_direction);
  Color picking_color = picking_manager_->GetPickableColor(pickable, GetBatcherId());
  Quad box = MakeBox(pos, size);
  batcher_->AddBox(box, z, colors, picking_color, nullptr);
}

void PrimitiveAssembler::AddTriangle(const Triangle& triangle, float z, const Color& color,
                                     std::unique_ptr<PickingUserData> user_data) {
  Color picking_color =
      PickingId::ToColor(PickingType::kTriangle, batcher_->GetNumElements(), GetBatcherId());

  AddTriangle(triangle, z, color, picking_color, std::move(user_data));
}

void PrimitiveAssembler::AddTriangle(const Triangle& triangle, float z, const Color& color,
                                     const std::shared_ptr<Pickable>& pickable) {
  ORBIT_CHECK(picking_manager_ != nullptr);

  Color picking_color = picking_manager_->GetPickableColor(pickable, GetBatcherId());

  AddTriangle(triangle, z, color, picking_color, nullptr);
}

void PrimitiveAssembler::AddTriangle(const Triangle& triangle, float z, const Color& color,
                                     const Color& picking_color,
                                     std::unique_ptr<PickingUserData> user_data) {
  std::array<Color, 3> colors;
  colors.fill(color);
  batcher_->AddTriangle(triangle, z, colors, picking_color, std::move(user_data));
}

// Draw a shaded trapezium with two sides parallel to the x-axis or y-axis.
void PrimitiveAssembler::AddShadedTrapezium(const Quad& trapezium, float z, const Color& color,
                                            std::unique_ptr<PickingUserData> user_data,
                                            ShadingDirection shading_direction) {
  std::array<Color, 4> colors;  // top_left, bottom_left, bottom_right, top_right.
  GetBoxGradientColors(color, &colors, shading_direction);
  Color picking_color =
      PickingId::ToColor(PickingType::kTriangle, batcher_->GetNumElements(), GetBatcherId());
  Triangle triangle_1{trapezium.vertices[0], trapezium.vertices[3], trapezium.vertices[1]};
  std::array<Color, 3> colors_1{colors[0], colors[1], colors[2]};
  batcher_->AddTriangle(triangle_1, z, colors_1, picking_color,
                        std::make_unique<PickingUserData>(*user_data));
  Triangle triangle_2{trapezium.vertices[3], trapezium.vertices[2], trapezium.vertices[1]};
  std::array<Color, 3> colors_2{colors[1], colors[2], colors[3]};
  batcher_->AddTriangle(triangle_2, z, colors_2, picking_color, std::move(user_data));
}

void PrimitiveAssembler::AddCircle(const Vec2& position, float radius, float z,
                                   const Color& color) {
  std::vector<Vec2> circle_points_scaled_by_radius;
  for (auto& point : circle_points) {
    circle_points_scaled_by_radius.emplace_back(radius * point);
  }

  Vec2 prev_point(position[0], position[1] - radius);
  Vec2 point_0(position[0], position[1]);
  for (size_t i = 0; i < circle_points_scaled_by_radius.size(); ++i) {
    Vec2 new_point(position[0] + circle_points_scaled_by_radius[i][0],
                   position[1] - circle_points_scaled_by_radius[i][1]);
    Vec2 point_1(prev_point[0], prev_point[1]);
    Vec2 point_2(new_point[0], new_point[1]);
    Triangle triangle(point_0, point_1, point_2);
    AddTriangle(triangle, z, color);
    prev_point = new_point;
  }
}

void PrimitiveAssembler::AddVerticalArrow(Vec2 starting_pos, Vec2 arrow_body_size,
                                          Vec2 arrow_head_size, float z, const Color& arrow_color,
                                          ArrowDirection arrow_direction) {
  float body_head_meeting_y =
      starting_pos[1] +
      (arrow_direction == ArrowDirection::kUp ? -arrow_body_size[1] : arrow_body_size[1]);

  float head_half_width = arrow_head_size[0] / 2;
  float head_length = arrow_head_size[1];
  float tip_of_head_y =
      body_head_meeting_y + (head_length * (arrow_direction == ArrowDirection::kUp ? -1 : 1));

  Triangle arrow_head{
      Vec2(starting_pos[0], tip_of_head_y),
      Vec2(starting_pos[0] - head_half_width, body_head_meeting_y),
      Vec2(starting_pos[0] + head_half_width, body_head_meeting_y),
  };
  AddTriangle(arrow_head, z, arrow_color);

  float arrow_body_min_y = std::min(starting_pos[1], body_head_meeting_y);
  float arrow_body_max_y = std::max(starting_pos[1], body_head_meeting_y);
  float body_half_width = arrow_body_size[0] / 2;

  std::array<Vec2, 4> box_vertices{
      Vec2(starting_pos[0] - body_half_width, arrow_body_max_y),
      Vec2(starting_pos[0] - body_half_width, arrow_body_min_y),
      Vec2(starting_pos[0] + body_half_width, arrow_body_min_y),
      Vec2(starting_pos[0] + body_half_width, arrow_body_max_y),
  };
  Quad arrow_body(box_vertices);
  AddBox(arrow_body, z, arrow_color);
}

void PrimitiveAssembler::AddQuadBorder(const Quad& quad, float z, const Color& color,
                                       std::unique_ptr<orbit_gl::PickingUserData> user_data) {
  AddLine(quad.vertices[0], quad.vertices[1], z, color,
          std::make_unique<PickingUserData>(*user_data));
  AddLine(quad.vertices[1], quad.vertices[2], z, color,
          std::make_unique<PickingUserData>(*user_data));
  AddLine(quad.vertices[2], quad.vertices[3], z, color,
          std::make_unique<PickingUserData>(*user_data));
  AddLine(quad.vertices[3], quad.vertices[0], z, color, std::move(user_data));
}

void PrimitiveAssembler::AddQuadBorder(const Quad& quad, float z, const Color& color) {
  AddLine(quad.vertices[0], quad.vertices[1], z, color);
  AddLine(quad.vertices[1], quad.vertices[2], z, color);
  AddLine(quad.vertices[2], quad.vertices[3], z, color);
  AddLine(quad.vertices[3], quad.vertices[0], z, color);
}

void PrimitiveAssembler::GetBoxGradientColors(const Color& color, std::array<Color, 4>* colors,
                                              ShadingDirection shading_direction) {
  constexpr float kGradientCoeff = 0.94f;
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

void PrimitiveAssembler::StartNewFrame() { batcher_->ResetElements(); }

const orbit_client_protos::TimerInfo* PrimitiveAssembler::GetTimerInfo(PickingId id) const {
  const PickingUserData* data = GetUserData(id);

  if (data != nullptr && data->timer_info_ != nullptr) {
    return data->timer_info_;
  }

  return nullptr;
}

}  // namespace orbit_gl