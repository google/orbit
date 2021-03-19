// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_TRACK_H_
#define ORBIT_GL_TIMER_TRACK_H_

#include <stdint.h>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "BlockChain.h"
#include "CallstackThreadBar.h"
#include "CoreMath.h"
#include "OrbitClientData/CallstackTypes.h"
#include "PickingManager.h"
#include "ScopeTree.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "TimerChain.h"
#include "TracepointThreadBar.h"
#include "Track.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"

class OrbitApp;
class TextRenderer;

namespace internal {
struct DrawData {
  uint64_t min_tick;
  uint64_t max_tick;
  uint64_t highlighted_function_id;
  uint64_t ns_per_pixel;
  uint64_t min_timegraph_tick;
  Batcher* batcher;
  GlCanvas* canvas;
  const TextBox* selected_textbox;
  double inv_time_window;
  float world_start_x;
  float world_width;
  float z_offset;
  float z;
  bool is_collapsed;
};

struct Rect {
  Rect(float pos_x, float pos_y, float size_x, float size_y)
      : pos(pos_x, pos_y), size(size_x, size_y) {}
  Vec2 pos;
  Vec2 size;
};
}  // namespace internal

class TimerTrack : public Track {
 public:
  explicit TimerTrack(TimeGraph* time_graph, TimeGraphLayout* layout, OrbitApp* app,
                      const CaptureData* capture_data);
  ~TimerTrack() override = default;

  // Pickable
  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;
  virtual void OnTimer(const orbit_client_protos::TimerInfo& timer_info);
  [[nodiscard]] std::string GetTooltip() const override;

  // Track
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode /*picking_mode*/, float z_offset = 0) override;
  [[nodiscard]] Type GetType() const override { return kTimerTrack; }

  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetTimers() const override;
  [[nodiscard]] uint32_t GetDepth() const { return depth_; }
  [[nodiscard]] std::string GetExtraInfo(const orbit_client_protos::TimerInfo& timer);

  [[nodiscard]] const TextBox* GetFirstAfterTime(uint64_t time, uint32_t depth) const;
  [[nodiscard]] const TextBox* GetFirstBeforeTime(uint64_t time, uint32_t depth) const;

  // Must be overriden by child class for sensible behavior.
  [[nodiscard]] virtual const TextBox* GetLeft(const TextBox* textbox) const { return textbox; };
  // Must be overriden by child class for sensible behavior.
  [[nodiscard]] virtual const TextBox* GetRight(const TextBox* textbox) const { return textbox; };

  [[nodiscard]] virtual const TextBox* GetUp(const TextBox* textbox) const;
  [[nodiscard]] virtual const TextBox* GetDown(const TextBox* textbox) const;

  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllChains() const override;
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllSerializableChains() const override;
  [[nodiscard]] bool IsEmpty() const override;

  [[nodiscard]] bool IsCollapsible() const override { return depth_ > 1; }

  virtual void UpdateBoxHeight();
  [[nodiscard]] virtual float GetTextBoxHeight(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const;
  [[nodiscard]] virtual float GetYFromTimer(const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] float GetYFromDepth(uint32_t depth) const;

  [[nodiscard]] virtual float GetHeaderHeight() const;

  [[nodiscard]] int GetVisiblePrimitiveCount() const override { return visible_timer_count_; }

 protected:
  [[nodiscard]] virtual bool IsTimerActive(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return true;
  }
  [[nodiscard]] virtual Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                            bool is_selected, bool is_highlighted) const = 0;
  [[nodiscard]] virtual bool TimerFilter(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return true;
  }

  [[nodiscard]] bool DrawTimer(const TextBox* prev_text_box, const TextBox* next_text_box,
                               const internal::DrawData& draw_data, TextBox* current_text_box,
                               uint64_t* min_ignore, uint64_t* max_ignore);

  void UpdateDepth(uint32_t depth) {
    if (depth > depth_) depth_ = depth;
  }
  [[nodiscard]] std::shared_ptr<TimerChain> GetTimers(uint32_t depth) const;

  virtual void SetTimesliceText(const orbit_client_protos::TimerInfo& /*timer*/, float /*min_x*/,
                                float /*z_offset*/, TextBox* /*text_box*/) {}

  [[nodiscard]] static internal::DrawData GetDrawData(uint64_t min_tick, uint64_t max_tick,
                                                      float z_offset, Batcher* batcher,
                                                      TimeGraph* time_graph, bool is_collapsed,
                                                      const TextBox* selected_textbox,
                                                      uint64_t highlighted_function_id);

  TextRenderer* text_renderer_ = nullptr;
  uint32_t depth_ = 0;
  mutable absl::Mutex mutex_;
  std::map<int, std::shared_ptr<TimerChain>> timers_;
  int visible_timer_count_ = 0;

  [[nodiscard]] virtual std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const;
  float GetHeight() const override;
  float box_height_ = 0.0f;

  static const Color kHighlightColor;

  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_TIMER_TRACK_H_
