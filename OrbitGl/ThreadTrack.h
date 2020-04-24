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

//-----------------------------------------------------------------------------
class ThreadTrack : public Track {
 public:
  ThreadTrack(TimeGraph* a_TimeGraph, uint32_t a_ThreadID);
  ~ThreadTrack() override = default;

  // Pickable
  void Draw(GlCanvas* a_Canvas, bool a_Picking) override;
  void OnDrag(int a_X, int a_Y) override;
  void OnTimer(const Timer& a_Timer);

  // Track
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) override;
  Type GetType() const override { return kThreadTrack; }
  float GetHeight() const override;

  std::vector<std::shared_ptr<TimerChain>> GetTimers() override;
  uint32_t GetDepth() const { return m_Depth; }

  Color GetColor() const;
  static Color GetColor(ThreadID a_TID);
  uint32_t GetNumTimers() const { return m_NumTimers; }
  TickType GetMinTime() const { return m_MinTime; }
  TickType GetMaxTime() const { return m_MaxTime; }

  const TextBox* GetFirstAfterTime(TickType a_Tick, uint32_t a_Depth) const;
  const TextBox* GetFirstBeforeTime(TickType a_Tick, uint32_t a_Depth) const;

  const TextBox* GetLeft(TextBox* a_TextBox) const;
  const TextBox* GetRight(TextBox* a_TextBox) const;
  const TextBox* GetUp(TextBox* a_TextBox) const;
  const TextBox* GetDown(TextBox* a_TextBox) const;

  std::vector<std::shared_ptr<TimerChain>> GetAllChains() override ;

  void SetEventTrackColor(Color color);

 protected:
  void UpdateDepth(uint32_t a_Depth) {
    if (a_Depth > m_Depth) m_Depth = a_Depth;
  }
  std::shared_ptr<TimerChain> GetTimers(uint32_t a_Depth) const;

 protected:
  TextRenderer* m_TextRenderer = nullptr;
  std::shared_ptr<EventTrack> m_EventTrack;
  uint32_t m_Depth = 0;
  ThreadID m_ThreadID;

  mutable Mutex m_Mutex;

  std::map<int, std::shared_ptr<TimerChain>> m_Timers;
};
