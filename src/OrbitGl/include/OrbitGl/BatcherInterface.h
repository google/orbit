// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_INTERFACE_H_
#define ORBIT_GL_BATCHER_INTERFACE_H_

#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

struct PickingUserData {
  using TooltipCallback = std::function<std::string(PickingId)>;
  const orbit_client_protos::TimerInfo* timer_info_;
  TooltipCallback generate_tooltip_;
  const void* custom_data_ = nullptr;

  explicit PickingUserData(const orbit_client_protos::TimerInfo* timer_info = nullptr,
                           TooltipCallback generate_tooltip = nullptr)
      : timer_info_(timer_info), generate_tooltip_(std::move(generate_tooltip)) {}
};

// Collects primitives to be rendered at a later point in time.
//
// BatcherInterface is an interface class. By calling BatcherInterface::AddXXX, primitives are
// added to internal CPU buffers, and sorted into layers formed by equal z-coordinates. Each layer
// should then be drawn seperately with BatcherInterface::DrawLayer(). BatcherInterface also
// provides a method to get the user data given a picking_id (in general used for tooltips).
class BatcherInterface {
 public:
  virtual ~BatcherInterface() = default;

  virtual void ResetElements() = 0;
  virtual void AddLine(Vec2 from, Vec2 to, float z, const Color& color, const Color& picking_color,
                       std::unique_ptr<PickingUserData> user_data) = 0;
  virtual void AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
                      const Color& picking_color, std::unique_ptr<PickingUserData> user_data) = 0;
  virtual void AddTriangle(const Triangle& triangle, float z, const std::array<Color, 3>& colors,
                           const Color& picking_color,
                           std::unique_ptr<PickingUserData> user_data) = 0;
  [[nodiscard]] virtual uint32_t GetNumElements() const = 0;

  [[nodiscard]] virtual std::vector<float> GetLayers() const = 0;
  virtual void DrawLayer(float layer, bool picking) = 0;

  [[nodiscard]] virtual const PickingUserData* GetUserData(PickingId id) const = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_BATCHER_INTERFACE_H_
