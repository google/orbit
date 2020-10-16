// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DELTA_TRACK_H_
#define ORBIT_GL_DELTA_TRACK_H_

#include "TimerTrack.h"

class FrameTrack : public TimerTrack {
 public:
  explicit FrameTrack(TimeGraph* time_graph, const orbit_client_protos::FunctionInfo& function);
  [[nodiscard]] Type GetType() const override { return kFrameTrack; }
  [[nodiscard]] bool IsCollapsable() const override { return maximum_box_ratio_ > 0.f; }

  [[nodiscard]] virtual float GetYFromDepth(uint32_t depth) const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  // Some values needed for visualization have to be computed after all timers have been added to
  // the track. This can be done using the Finalize() method.
  void Finalize();

  [[nodiscard]] float GetTextBoxHeight(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] float GetHeaderHeight() const override;

  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, double elapsed_us, float min_x,
                        TextBox* text_box) override;
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] std::string GetBoxTooltip(PickingId id) const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;

  void UpdateBoxHeight() override;

 protected:
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected) const override;
  [[nodiscard]] float GetHeight() const override;

 private:
  [[nodiscard]] float GetMaximumBoxHeight() const;
  [[nodiscard]] float GetAverageBoxHeight() const;

  orbit_client_protos::FunctionInfo function_;
  orbit_client_protos::FunctionStats stats_;

  std::vector<uint64_t> durations_;

  float maximum_box_ratio_ = 0.f;
};

#endif  // ORBIT_GL_THREAD_TRACK_H_
