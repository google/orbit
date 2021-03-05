// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef ORBIT_GL_SCHEDULER_TRACK_H_
#define ORBIT_GL_SCHEDULER_TRACK_H_

#include <string>

#include "CoreMath.h"
#include "EventTrack.h"
#include "PickingManager.h"
#include "TimerTrack.h"
#include "Track.h"
#include "capture_data.pb.h"

class OrbitApp;

class SchedulerTrack final : public TimerTrack {
 public:
  explicit SchedulerTrack(TimeGraph* time_graph, TimeGraphLayout* layout, OrbitApp* app,
                          const CaptureData* capture_data);
  ~SchedulerTrack() override = default;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] Type GetType() const override { return kSchedulerTrack; }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] bool IsCollapsible() const override { return false; }

  void UpdateBoxHeight() override;
  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;

 protected:
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected, bool is_highlighted) const override;
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;

 private:
  uint32_t num_cores_;
};

#endif  // ORBIT_GL_SCHEDULER_TRACK_H_
