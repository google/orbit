// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SCHEDULER_TRACK_H_
#define ORBIT_GL_SCHEDULER_TRACK_H_

#include "TimerTrack.h"
#include "capture_data.pb.h"

class SchedulerTrack : public TimerTrack {
 public:
  explicit SchedulerTrack(TimeGraph* time_graph);
  ~SchedulerTrack() override = default;

  [[nodiscard]] Type GetType() const override { return kSchedulerTrack; }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] bool IsCollapsable() const override { return false; }

  void UpdateBoxHeight() override;
  [[nodiscard]] float GetYFromDepth(uint32_t depth) const override;

 protected:
  [[nodiscard]] bool IsTimerActive(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                      bool is_selected) const override;
  [[nodiscard]] std::string GetBoxTooltip(PickingID id) const override;
};

#endif  // ORBIT_GL_SCHEDULER_TRACK_H_
