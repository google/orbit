// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FRAME_TRACK_H_
#define ORBIT_GL_FRAME_TRACK_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "CoreMath.h"
#include "EventTrack.h"
#include "PickingManager.h"
#include "TextBox.h"
#include "TimerChain.h"
#include "TimerTrack.h"
#include "Track.h"
#include "capture_data.pb.h"

class OrbitApp;

class FrameTrack : public TimerTrack {
 public:
  explicit FrameTrack(TimeGraph* time_graph, uint64_t function_id,
                      orbit_client_protos::FunctionInfo function, OrbitApp* app,
                      CaptureData* capture_data);
  [[nodiscard]] Type GetType() const override { return kFrameTrack; }
  [[nodiscard]] uint64_t GetFunctionId() const { return function_id_; }
  [[nodiscard]] bool IsCollapsible() const override { return GetMaximumScaleFactor() > 0.f; }

  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] float GetTextBoxHeight(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] float GetHeaderHeight() const override;

  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, double elapsed_us, float min_x,
                        float z_offset, TextBox* text_box) override;
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] std::string GetBoxTooltip(PickingId id) const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;

  void UpdateBoxHeight() override;

  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllSerializableChains() const override;

 protected:
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected) const override;
  [[nodiscard]] float GetHeight() const override;

 private:
  [[nodiscard]] float GetMaximumScaleFactor() const;
  [[nodiscard]] float GetMaximumBoxHeight() const;
  [[nodiscard]] float GetAverageBoxHeight() const;

  uint64_t function_id_;
  orbit_client_protos::FunctionInfo function_;
  orbit_client_protos::FunctionStats stats_;
};

#endif  // ORBIT_GL_FRAME_TRACK_H_
