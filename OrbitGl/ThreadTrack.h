// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_TRACK_H_
#define ORBIT_GL_THREAD_TRACK_H_

#include <map>
#include <memory>

#include "TimerTrack.h"
#include "capture_data.pb.h"

class ThreadTrack : public TimerTrack {
 public:
  ThreadTrack(TimeGraph* time_graph, int32_t thread_id);

  [[nodiscard]] int32_t GetThreadId() const { return thread_id_; }

  [[nodiscard]] Type GetType() const override { return kThreadTrack; }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] const TextBox* GetLeft(const TextBox* textbox) const override;
  [[nodiscard]] const TextBox* GetRight(const TextBox* textbox) const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;

  void UpdateBoxHeight() override;
  void SetEventTrackColor(Color color);
  [[nodiscard]] bool IsEmpty() const override;

  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) override;

 protected:
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer,
                                    bool is_selected) const override;
  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, double elapsed_us, float min_x,
                        TextBox* text_box) override;
  [[nodiscard]] std::string GetBoxTooltip(PickingId id) const override;

  std::shared_ptr<EventTrack> event_track_;
};

#endif  // ORBIT_GL_THREAD_TRACK_H_
