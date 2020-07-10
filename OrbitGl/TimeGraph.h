// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <unordered_map>
#include <utility>

#include "Batcher.h"
#include "BlockChain.h"
#include "ContextSwitch.h"
#include "Core.h"
#include "EventBuffer.h"
#include "Geometry.h"
#include "GpuTrack.h"
#include "SchedulerTrack.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "absl/container/flat_hash_map.h"

class TimeGraph {
 public:
  TimeGraph();

  void Draw(GlCanvas* canvas, bool a_Picking = false);
  void DrawTracks(GlCanvas* canvas, bool a_Picking = false);
  void DrawText(GlCanvas* canvas);

  void NeedsUpdate();
  void UpdatePrimitives();
  void SortTracks();
  std::vector<CallstackEvent> SelectEvents(float a_WorldStart, float a_WorldEnd,
                                           ThreadID a_TID);

  void ProcessTimer(const Timer& a_Timer);
  void UpdateMaxTimeStamp(TickType a_Time);

  float GetThreadTotalHeight();
  float GetTextBoxHeight() const { return m_Layout.GetTextBoxHeight(); }
  int GetMarginInPixels() const { return m_Margin; }
  float GetWorldFromTick(TickType a_Time) const;
  float GetWorldFromUs(double a_Micros) const;
  TickType GetTickFromWorld(float a_WorldX);
  TickType GetTickFromUs(double a_MicroSeconds) const;
  double GetUsFromTick(TickType time) const;
  double GetTimeWindowUs() const { return m_TimeWindowUs; }
  void GetWorldMinMax(float& a_Min, float& a_Max) const;
  bool UpdateSessionMinMaxCounter();

  void Clear();
  void ZoomAll();
  void Zoom(const TextBox* a_TextBox);
  void ZoomTime(float a_ZoomValue, double a_MouseRatio);
  void SetMinMax(double a_MinTimeUs, double a_MaxTimeUs);
  void PanTime(int a_InitialX, int a_CurrentX, int a_Width,
               double a_InitialTime);
  double GetTime(double a_Ratio);
  double GetTimeIntervalMicro(double a_Ratio);
  void Select(const TextBox* a_TextBox) { SelectRight(a_TextBox); }
  void SelectLeft(const TextBox* a_TextBox);
  void SelectRight(const TextBox* a_TextBox);
  double GetSessionTimeSpanUs();
  double GetCurrentTimeSpanUs();
  void NeedsRedraw() { m_NeedsRedraw = true; }
  bool IsRedrawNeeded() const { return m_NeedsRedraw; }
  void ToggleDrawText() { m_DrawText = !m_DrawText; }
  void SetThreadFilter(const std::string& a_Filter);

  bool IsVisible(const Timer& a_Timer);
  int GetNumDrawnTextBoxes() { return m_NumDrawnTextBoxes; }
  void SetPickingManager(class PickingManager* a_Manager) {
    m_PickingManager = a_Manager;
  }
  void SetTextRenderer(TextRenderer* a_TextRenderer) {
    m_TextRenderer = a_TextRenderer;
  }
  TextRenderer* GetTextRenderer() { return &m_TextRendererStatic; }
  void SetStringManager(std::shared_ptr<StringManager> str_manager);
  void SetCanvas(GlCanvas* a_Canvas);
  GlCanvas* GetCanvas() { return m_Canvas; }
  void SetFontSize(int a_FontSize);
  Batcher& GetBatcher() { return m_Batcher; }
  uint32_t GetNumTimers() const;
  uint32_t GetNumCores() const;
  std::vector<std::shared_ptr<TimerChain>> GetAllTimerChains() const;
  std::vector<std::shared_ptr<TimerChain>> GetAllThreadTrackTimerChains() const;

  void OnDrag(float a_Ratio);
  double GetMinTimeUs() const { return m_MinTimeUs; }
  double GetMaxTimeUs() const { return m_MaxTimeUs; }
  const TimeGraphLayout& GetLayout() const { return m_Layout; }
  TimeGraphLayout& GetLayout() { return m_Layout; }
  float GetVerticalMargin() const { return vertical_margin_; }
  void SetVerticalMargin(float margin) { vertical_margin_ = margin; }

  void OnLeft();
  void OnRight();
  void OnUp();
  void OnDown();

  Color GetThreadColor(ThreadID tid) const;
  StringManager* GetStringManager() { return string_manager_.get(); }

 protected:
  uint64_t GetGpuTimelineHash(const Timer& timer) const;
  std::shared_ptr<SchedulerTrack> GetOrCreateSchedulerTrack();
  std::shared_ptr<ThreadTrack> GetOrCreateThreadTrack(ThreadID a_TID);
  std::shared_ptr<GpuTrack> GetOrCreateGpuTrack(uint64_t timeline_hash);

 private:
  TextRenderer m_TextRendererStatic;
  TextRenderer* m_TextRenderer = nullptr;
  GlCanvas* m_Canvas = nullptr;
  TextBox m_SceneBox;
  int m_NumDrawnTextBoxes = 0;

  double m_RefTimeUs = 0;
  double m_MinTimeUs = 0;
  double m_MaxTimeUs = 0;
  TickType m_SessionMinCounter = 0;
  TickType m_SessionMaxCounter = 0;
  std::map<ThreadID, uint32_t> m_EventCount;
  double m_TimeWindowUs = 0;
  float m_WorldStartX = 0;
  float m_WorldWidth = 0;
  float min_y_ = 0;
  int m_Margin = 0;
  float vertical_margin_ = 0;

  double m_ZoomValue = 0;
  double m_MouseRatio = 0;

  TimeGraphLayout m_Layout;

  std::map<ThreadID, uint32_t> m_ThreadCountMap;

  // Be careful when directly changing these members without using the
  // methods NeedsRedraw() or NeedsUpdate():
  // m_NeedsUpdatePrimitives should always imply m_NeedsRedraw, that is
  // m_NeedsUpdatePrimitives => m_NeedsRedraw is an invariant of this
  // class. When updating the primitives, which computes the primitives
  // to be drawn and their coordinates, we always have to redraw the
  // timeline.
  bool m_NeedsUpdatePrimitives = false;
  bool m_NeedsRedraw = false;

  bool m_DrawText = true;

  Batcher m_Batcher;
  PickingManager* m_PickingManager = nullptr;
  Timer m_LastThreadReorder;

  mutable Mutex m_Mutex;
  std::vector<std::shared_ptr<Track>> tracks_;
  std::unordered_map<ThreadID, std::shared_ptr<ThreadTrack>> thread_tracks_;
  // Mapping from timeline hash to GPU tracks.
  std::unordered_map<uint64_t, std::shared_ptr<GpuTrack>> gpu_tracks_;
  std::vector<std::shared_ptr<Track>> sorted_tracks_;
  std::string m_ThreadFilter;

  std::set<uint32_t> cores_seen_;
  std::shared_ptr<SchedulerTrack> scheduler_track_;
  std::shared_ptr<ThreadTrack> process_track_;

  std::shared_ptr<StringManager> string_manager_;
};

extern TimeGraph* GCurrentTimeGraph;
