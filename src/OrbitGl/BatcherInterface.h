// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_INTERFACE_H_
#define ORBIT_GL_BATCHER_INTERFACE_H_

#include "ClientProtos/capture_data.pb.h"
#include "Geometry.h"
#include "PickingManager.h"
#include "TranslationStack.h"

using TooltipCallback = std::function<std::string(PickingId)>;

namespace orbit_gl {

struct PickingUserData {
  const orbit_client_protos::TimerInfo* timer_info_;
  TooltipCallback generate_tooltip_;
  const void* custom_data_ = nullptr;

  explicit PickingUserData(const orbit_client_protos::TimerInfo* timer_info = nullptr,
                           TooltipCallback generate_tooltip = nullptr)
      : timer_info_(timer_info), generate_tooltip_(std::move(generate_tooltip)) {}
};

// Collects primitives to be rendered at a later point in time.
//
// By calling BatcherInterface::AddXXX, primitives are added to internal CPU buffers, and sorted
// into layers formed by equal z-coordinates. Each layer should then be drawn seperately with
// BatcherInterface::DrawLayer(). BatcherInterface also provides a method to get the user data
// given a picking_id (in general used for tooltips).
//
// BatcherInterface is a pure interface class except for Translations and BatcherId.
class BatcherInterface {
 public:
  BatcherInterface(BatcherId batcher_id) : batcher_id_(batcher_id) {}
  virtual void ResetElements() = 0;
  virtual void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
                       std::unique_ptr<PickingUserData> user_data) = 0;
  virtual void AddBox(const Box& box, const std::array<Color, 4>& colors,
                      const Color& picking_color, std::unique_ptr<PickingUserData> user_data) = 0;
  virtual void AddTriangle(const Triangle& triangle, const std::array<Color, 3>& colors,
                           const Color& picking_color,
                           std::unique_ptr<PickingUserData> user_data) = 0;
  [[nodiscard]] virtual uint32_t GetNumElements() const = 0;

  [[nodiscard]] virtual std::vector<float> GetLayers() const = 0;
  virtual void DrawLayer(float layer, bool picking) const = 0;

  [[nodiscard]] virtual const PickingUserData* GetUserData(PickingId id) const = 0;

  [[nodiscard]] BatcherId GetBatcherId() const { return batcher_id_; }
  void PushTranslation(float x, float y, float z = 0.f) { translations_.PushTranslation(x, y, z); }
  void PopTranslation() { translations_.PopTranslation(); }

 protected:
  orbit_gl::TranslationStack translations_;

 private:
  BatcherId batcher_id_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_BATCHER_INTERFACE_H_
