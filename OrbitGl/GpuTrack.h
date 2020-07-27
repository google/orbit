// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_TRACK_H_
#define ORBIT_GL_GPU_TRACK_H_

#include <map>
#include <memory>

#include "BlockChain.h"
#include "CallstackTypes.h"
#include "StringManager.h"
#include "TextBox.h"
#include "Threading.h"
#include "Track.h"

class TextRenderer;

// Maps the Linux kernel timeline names (like "gfx", "sdma0") to a more
// descriptive human readable form that is used for the track labe.
std::string MapGpuTimelineToTrackLabel(std::string_view timeline);

class GpuTrack : public Track {
 public:
  GpuTrack(TimeGraph* time_graph, std::shared_ptr<StringManager> string_manager,
           uint64_t timeline_hash);
  ~GpuTrack() override = default;

  // Pickable
  void Draw(GlCanvas* canvas, bool picking) override;
  void OnDrag(int x, int y) override;
  void OnTimer(const Timer& timer);
  std::string GetTooltip() const override;

  // Track
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) override;
  Type GetType() const override { return kGpuTrack; }
  float GetHeight() const override;

  std::vector<std::shared_ptr<TimerChain>> GetTimers() override;
  uint32_t GetDepth() const { return depth_; }
  std::string GetExtraInfo(const Timer& timer);

  Color GetColor() const;
  uint32_t GetNumTimers() const { return num_timers_; }
  TickType GetMinTime() const { return min_time_; }
  TickType GetMaxTime() const { return max_time_; }

  const TextBox* GetFirstAfterTime(TickType time, uint32_t depth) const;
  const TextBox* GetFirstBeforeTime(TickType time, uint32_t depth) const;

  const TextBox* GetLeft(TextBox* textbox) const;
  const TextBox* GetRight(TextBox* textbox) const;
  const TextBox* GetUp(TextBox* textbox) const;
  const TextBox* GetDown(TextBox* textbox) const;

  std::vector<std::shared_ptr<TimerChain>> GetAllChains() override;
  bool IsCollapsable() const override { return depth_ > 1; }

 protected:
  void UpdateDepth(uint32_t depth) {
    if (depth > depth_) depth_ = depth;
  }
  std::shared_ptr<TimerChain> GetTimers(uint32_t depth) const;

 private:
  Color GetTimerColor(const Timer& timer, bool is_selected,
                      bool inactive) const;
  void SetTimesliceText(const Timer& timer, double elapsed_us, float min_x,
                        TextBox* text_box);


 protected:
  TextRenderer* text_renderer_ = nullptr;
  uint32_t depth_ = 0;
  uint64_t timeline_hash_;
  mutable Mutex mutex_;
  std::map<int, std::shared_ptr<TimerChain>> timers_;

  std::shared_ptr<StringManager> string_manager_;

  std::string GetBoxTooltip(PickingID id) const;
  std::string GetSwQueueTooltip(const Timer& timer) const;
  std::string GetHwQueueTooltip(const Timer& timer) const;
  std::string GetHwExecutionTooltip(const Timer& timer) const;
};

#endif  // ORBIT_GL_GPU_TRACK_H_
