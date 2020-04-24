//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <map>
#include <memory>

#include "BlockChain.h"
#include "CallstackTypes.h"
#include "TextBox.h"
#include "Threading.h"
#include "Track.h"

class TextRenderer;
class EventTrack;

class ThreadTrack : public Track {
 public:
  ThreadTrack(TimeGraph* time_graph, uint32_t thread_id);
  ~ThreadTrack() override = default;

  // Pickable
  void Draw(GlCanvas* canvas, bool picking) override;
  void OnDrag(int x, int y) override;
  void OnTimer(const Timer& timer);

  // Track
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) override;
  Type GetType() const override { return kThreadTrack; }
  float GetHeight() const override;

  std::vector<std::shared_ptr<TimerChain>> GetTimers() override;
  uint32_t GetDepth() const { return depth_; }

  Color GetColor() const;
  static Color GetColor(ThreadID a_TID);
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

  void SetEventTrackColor(Color color);

 protected:
  void UpdateDepth(uint32_t depth) {
    if (depth > depth_) depth_ = depth;
  }
  std::shared_ptr<TimerChain> GetTimers(uint32_t depth) const;

 protected:
  TextRenderer* text_renderer_ = nullptr;
  std::shared_ptr<EventTrack> event_track_;
  uint32_t depth_ = 0;
  ThreadID thread_id_;
  mutable Mutex mutex_;
  std::map<int, std::shared_ptr<TimerChain>> timers_;
};
