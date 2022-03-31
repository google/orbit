// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_H_
#define ORBIT_GL_BATCHER_H_

#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientProtos/capture_data.pb.h"
#include "CoreMath.h"
#include "Geometry.h"
#include "PickingManager.h"
#include "TranslationStack.h"

namespace orbit_gl {

using TooltipCallback = std::function<std::string(PickingId)>;

struct PickingUserData {
  const orbit_client_protos::TimerInfo* timer_info_;
  TooltipCallback generate_tooltip_;
  const void* custom_data_ = nullptr;

  explicit PickingUserData(const orbit_client_protos::TimerInfo* timer_info = nullptr,
                           TooltipCallback generate_tooltip = nullptr)
      : timer_info_(timer_info), generate_tooltip_(std::move(generate_tooltip)) {}
};

enum class ShadingDirection { kLeftToRight, kRightToLeft, kTopToBottom, kBottomToTop };

/**
Collects primitives to be rendered at a later point in time.

By calling Batcher::AddXXX, primitives are added to internal CPU buffers, and sorted
into layers formed by equal z-coordinates. Each layer can then be drawn seperately with
Batcher::DrawLayer(), or all layers can be drawn at once in their correct order using
Batcher::Draw():

NOTE: Batcher has a few pure virtual functions that have to be implemented: A few
AddInternalMethods, ResetElements(), GetLayers() and DrawLayers().
**/
class Batcher {
 public:
  explicit Batcher(BatcherId batcher_id, PickingManager* picking_manager = nullptr)
      : batcher_id_(batcher_id), picking_manager_(picking_manager) {
    constexpr int32_t kSteps = 22;
    const float angle = (kPiFloat * 2.f) / kSteps;
    for (int32_t i = 1; i <= kSteps; i++) {
      float new_x = sinf(angle * i);
      float new_y = cosf(angle * i);
      circle_points.emplace_back(Vec2(new_x, new_y));
    }
  }

  Batcher() = delete;
  Batcher(const Batcher&) = delete;
  Batcher(Batcher&&) = delete;

  void PushTranslation(float x, float y, float z = 0.f) { translations_.PushTranslation(x, y, z); }
  void PopTranslation() { translations_.PopTranslation(); }

  void AddLine(Vec2 from, Vec2 to, float z, const Color& color,
               std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                       std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddLine(Vec2 from, Vec2 to, float z, const Color& color, std::shared_ptr<Pickable> pickable);
  void AddVerticalLine(Vec2 pos, float size, float z, const Color& color,
                       std::shared_ptr<Pickable> pickable);

  void AddBox(const Tetragon& box, const std::array<Color, 4>& colors,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Tetragon& box, const Color& color,
              std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddBox(const Tetragon& box, const Color& color, std::shared_ptr<Pickable> pickable);

  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    ShadingDirection shading_direction);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    std::unique_ptr<PickingUserData> user_data,
                    ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddShadedBox(Vec2 pos, Vec2 size, float z, const Color& color,
                    std::shared_ptr<Pickable> pickable,
                    ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddRoundedBox(Vec2 pos, Vec2 size, float z, float radius, const Color& color,
                     float margin = 0);

  void AddBottomLeftRoundedCorner(Vec2 pos, float radius, float z, const Color& color);
  void AddTopLeftRoundedCorner(Vec2 pos, float radius, float z, const Color& color);
  void AddTopRightRoundedCorner(Vec2 pos, float radius, float z, const Color& color);
  void AddBottomRightRoundedCorner(Vec2 pos, float radius, float z, const Color& color);

  void AddTriangle(const Triangle& triangle, const Color& color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);
  void AddShadedTrapezium(const Tetragon& trapezium, const Color& color,
                          std::unique_ptr<PickingUserData> user_data = nullptr,
                          ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  void AddTriangle(const Triangle& triangle, const Color& color,
                   std::shared_ptr<Pickable> pickable);

  void AddCircle(const Vec2& position, float radius, float z, Color color);

  [[nodiscard]] virtual std::vector<float> GetLayers() const = 0;
  virtual void DrawLayer(float layer, bool picking) const = 0;

  void Draw(bool picking = false) const;

  void StartNewFrame();

  [[nodiscard]] PickingManager* GetPickingManager() const { return picking_manager_; }
  void SetPickingManager(PickingManager* picking_manager) { picking_manager_ = picking_manager; }

  [[nodiscard]] const PickingUserData* GetUserData(PickingId id) const;
  [[nodiscard]] PickingUserData* GetUserData(PickingId id);

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetTimerInfo(PickingId id) const;

  static constexpr uint32_t kNumArcSides = 16;

 protected:
  void AddTriangle(const Triangle& triangle, const Color& color, const Color& picking_color,
                   std::unique_ptr<PickingUserData> user_data = nullptr);
  orbit_gl::TranslationStack translations_;
  std::vector<std::unique_ptr<PickingUserData>> user_data_;

 private:
  void GetBoxGradientColors(const Color& color, std::array<Color, 4>* colors,
                            ShadingDirection shading_direction = ShadingDirection::kLeftToRight);
  virtual void ResetElements() = 0;
  virtual void AddLineInternal(Vec2 from, Vec2 to, float z, const Color& color,
                               const Color& picking_color,
                               std::unique_ptr<PickingUserData> user_data) = 0;
  virtual void AddBoxInternal(const Tetragon& box, const std::array<Color, 4>& colors,
                              const Color& picking_color,
                              std::unique_ptr<PickingUserData> user_data) = 0;
  virtual void AddTriangleInternal(const Triangle& triangle, const std::array<Color, 3>& colors,
                                   const Color& picking_color,
                                   std::unique_ptr<PickingUserData> user_data) = 0;

  BatcherId batcher_id_;
  PickingManager* picking_manager_;

  std::vector<Vec2> circle_points;
};

}  // namespace orbit_gl

#endif
