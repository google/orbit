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

typedef BlockChain<TextBox, 4 * 1024> TimerChain;

//-----------------------------------------------------------------------------
class ThreadTrack : public Track {
 public:
  ThreadTrack(TimeGraph* a_TimeGraph, uint32_t a_ThreadID);

  // Pickable
  void Draw(GlCanvas* a_Canvas, bool a_Picking) override;
  void OnDrag(int a_X, int a_Y) override;
  void OnTimer(const Timer& a_Timer);

  // Track
  float GetHeight() const override;

  std::vector<std::shared_ptr<TimerChain>> GetTimers();
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

  std::vector<std::shared_ptr<TimerChain>> GetAllChains() const;

  bool GetVisible() const { return m_Visible; }
  void SetVisible(bool value) { m_Visible = value; }

 protected:
  inline void UpdateDepth(uint32_t a_Depth) {
    if (a_Depth > m_Depth) m_Depth = a_Depth;
  }
  std::shared_ptr<TimerChain> GetTimers(uint32_t a_Depth) const;

 protected:
  TextRenderer* m_TextRenderer = nullptr;
  std::shared_ptr<EventTrack> m_EventTrack;
  uint32_t m_Depth = 1;
  ThreadID m_ThreadID;
  bool m_Visible = true;

  std::atomic<uint32_t> m_NumTimers;
  std::atomic<TickType> m_MinTime;
  std::atomic<TickType> m_MaxTime;
  mutable Mutex m_Mutex;

  std::map<int, std::shared_ptr<TimerChain>> m_Timers;
};
