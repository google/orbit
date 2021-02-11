// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_TRACK_H_
#define ORBIT_GL_THREAD_TRACK_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <string>

#include "CoreMath.h"
#include "EventTrack.h"
#include "PickingManager.h"
#include "TextBox.h"
#include "ThreadStateTrack.h"
#include "TimerTrack.h"
#include "TracepointTrack.h"
#include "Track.h"
#include "capture_data.pb.h"

class OrbitApp;

class ThreadTrack final : public TimerTrack {
 public:
  explicit ThreadTrack(TimeGraph* time_graph, TimeGraphLayout* layout, int32_t thread_id,
                       OrbitApp* app, const CaptureData* capture_data);
  void InitializeNameAndLabel(int32_t thread_id);

  [[nodiscard]] int32_t GetThreadId() const { return thread_id_; }

  [[nodiscard]] Type GetType() const override { return kThreadTrack; }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] const TextBox* GetLeft(const TextBox* textbox) const override;
  [[nodiscard]] const TextBox* GetRight(const TextBox* textbox) const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;

  void OnPick(int x, int y) override;

  void UpdateBoxHeight() override;
  void SetTrackColor(Color color);
  [[nodiscard]] bool IsEmpty() const override;

  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;

 protected:
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] bool IsTrackSelected() const override;

  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer, bool is_selected,
                                    bool is_highlighted) const override;
  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, double elapsed_us, float min_x,
                        float z_offset, TextBox* text_box) override;
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;

  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] float GetHeaderHeight() const override;

  void UpdatePositionOfSubtracks();
  void UpdateMinMaxTimestamps();

  std::shared_ptr<orbit_gl::ThreadStateTrack> thread_state_track_;
  std::shared_ptr<orbit_gl::EventTrack> event_track_;
  std::shared_ptr<orbit_gl::TracepointTrack> tracepoint_track_;
};

#endif  // ORBIT_GL_THREAD_TRACK_H_
