// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRIMITIVE_ASSEMBLER_H_
#define ORBIT_GL_PRIMITIVE_ASSEMBLER_H_

#include <stdint.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/Batcher.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

enum class ShadingDirection { kLeftToRight, kRightToLeft, kTopToBottom, kBottomToTop };

/**
Collects primitives to be rendered at a later point in time.

By calling PrimitiveAssembler::AddXXX, primitives are added to internal CPU buffers, and sorted
into layers formed by equal z-coordinates. Each layer can then be drawn seperately with
PrimitiveAssembler::DrawLayer(), or all layers can be drawn at once in their correct order using
PrimitiveAssembler::Draw():

NOTE: PrimitiveAssembler has a few pure virtual functions that have to be implemented: A few
AddInternalMethods, ResetElements(), GetLayers() and DrawLayers().
**/
class PrimitiveAssembler {
 public:
  static constexpr int32_t kCirclePoints = 22;
  explicit PrimitiveAssembler(Batcher* batcher, PickingManager* picking_manager = nullptr)
      : batcher_(batcher), picking_manager_(picking_manager) {
    ORBIT_CHECK(batcher_ != nullptr);

    const float angle = (kPiFloat * 2.f) / kCirclePoints;
    for (int32_t i = 1; i <= kCirclePoints; i++) {
      float new_x = std::sin(angle * i);
      float new_y = std::cos(angle * i);
      circle_points.emplace_back(Vec2(new_x, new_y));
    }
  }

  PrimitiveAssembler() = delete;
  PrimitiveAssembler(const PrimitiveAssembler&) = delete;
  PrimitiveAssembler(PrimitiveAssembler&&) = delete;

  void PushTranslation(float x, float y, float z = 0.f) { batcher_->PushTranslation(x, y, z); }
  void PopTranslation() { batcher_->PopTranslation(); }

  void AddLine(const Vec2& from, const Vec2& to, float z, const Color& color,
               std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddVerticalLine(const Vec2& pos, float size, float z, const Color& color,
                       std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddLine(const Vec2& from, const Vec2& to, float z, const Color& color,
               const std::shared_ptr<Pickable>& pickable);
  void AddVerticalLine(const Vec2& pos, float size, float z, const Color& color,
                       const std::shared_ptr<Pickable>& pickable);

  void AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Quad& box, float z, const Color& color,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Quad& box, float z, const Color& color,
              const std::shared_ptr<Pickable>& pickable);

  void AddShadedBox(const Vec2& pos, const Vec2& size, float z, const Color& color);
  void AddShadedBox(const Vec2& pos, const Vec2& size, float z, const Color& color,
                    ShadingDirection shading_direction);
  void AddShadedBox(const Vec2& pos, const Vec2& size, float z, const Color& color,
                    std::unique_ptr<PickingUserData> user_data,
                    ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddShadedBox(const Vec2& pos, const Vec2& size, float z, const Color& color,
                    const std::shared_ptr<Pickable>& pickable,
                    ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddRoundedBox(Vec2 pos, Vec2 size, float z, float radius, const Color& color,
                     float margin = 0);

  // TODO(b/227744958) This should probably be removed and AddBox should be used instead
  void AddShadedTrapezium(const Quad& trapezium, float z, const Color& color,
                          std::unique_ptr<PickingUserData> user_data,
                          ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddTriangle(const Triangle& triangle, float z, const Color& color,
                   const std::shared_ptr<Pickable>& pickable);
  void AddTriangle(const Triangle& triangle, float z, const Color& color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);

  void AddQuadBorder(const Quad& quad, float z, const Color& color,
                     std::unique_ptr<orbit_gl::PickingUserData> user_data);
  void AddQuadBorder(const Quad& quad, float z, const Color& color);
  void AddCircle(const Vec2& position, float radius, float z, const Color& color);

  enum class ArrowDirection { kUp, kDown };
  //
  //   ⸢¯¯¯¯¯⸣ = arrow_body_size[0]
  //   ⎡¯¯X¯¯⎤     ⎤
  //   |     |     |
  //   |     |     |  = arrow_body_size[1]
  //   |     |     ⎦
  //  \¯¯¯¯¯¯¯/  ⎤
  //   \     /   |
  //    \   /    | = arrow_head_size[1]
  //     \ /     ⎦
  //      ˅
  //  ⸤________⸥ = arrow_head_size[0]
  //
  // x is starting_pos
  void AddVerticalArrow(Vec2 starting_pos, Vec2 arrow_body_size, Vec2 arrow_head_size, float z,
                        const Color& arrow_color, ArrowDirection arrow_direction);

  void StartNewFrame();

  [[nodiscard]] PickingManager* GetPickingManager() const { return picking_manager_; }
  [[nodiscard]] const PickingUserData* GetUserData(PickingId id) const {
    return batcher_->GetUserData(id);
  }
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetTimerInfo(PickingId id) const;

  static constexpr uint32_t kNumArcSides = 16;

 private:
  [[nodiscard]] BatcherId GetBatcherId() const { return batcher_->GetBatcherId(); }

  void AddBottomLeftRoundedCorner(const Vec2& pos, float radius, float z, const Color& color);
  void AddTopLeftRoundedCorner(const Vec2& pos, float radius, float z, const Color& color);
  void AddTopRightRoundedCorner(const Vec2& pos, float radius, float z, const Color& color);
  void AddBottomRightRoundedCorner(const Vec2& pos, float radius, float z, const Color& color);
  void AddTriangle(const Triangle& triangle, float z, const Color& color,
                   const Color& picking_color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);

  static void GetBoxGradientColors(
      const Color& color, std::array<Color, 4>* colors,
      ShadingDirection shading_direction = ShadingDirection::kLeftToRight);

  Batcher* batcher_;
  PickingManager* picking_manager_;

  std::vector<Vec2> circle_points;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PRIMITIVE_ASSEMBLER_H_
