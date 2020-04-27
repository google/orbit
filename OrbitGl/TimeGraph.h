//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <unordered_map>
#include <utility>

#include "Batcher.h"
#include "BlockChain.h"
#include "ContextSwitch.h"
#include "Core.h"
#include "EventBuffer.h"
#include "Geometry.h"
#include "MemoryTracker.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "TimeGraphLayout.h"
#include "absl/container/flat_hash_map.h"

class Systrace;

class TimeGraph {
 public:
  TimeGraph();

  void Draw(bool a_Picking = false);
  void DrawTracks(bool a_Picking = false);
  void DrawMainFrame(TextBox& a_Box);
  void DrawLineBuffer(bool a_Picking);
  void DrawBoxBuffer(bool a_Picking);
  void DrawBuffered(bool a_Picking);
  void DrawText();

  void NeedsUpdate();
  void UpdatePrimitives();
  void SortTracks();
  std::vector<CallstackEvent> SelectEvents(float a_WorldStart, float a_WorldEnd,
                                           ThreadID a_TID);

  void ProcessTimer(const Timer& a_Timer);
  void UpdateThreadDepth(int a_ThreadId, int a_Depth);
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
  void AddContextSwitch(const ContextSwitch& a_CS);
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
  void SetSystrace(std::shared_ptr<Systrace> a_Systrace) {
    m_Systrace = std::move(a_Systrace);
  }
  Batcher& GetBatcher() { return m_Batcher; }
  uint32_t GetNumTimers() const;
  uint32_t GetNumCores() const;
  std::vector<std::shared_ptr<TimerChain>> GetAllTimerChains() const;

  void OnDrag(float a_Ratio);
  double GetMinTimeUs() const { return m_MinTimeUs; }
  double GetMaxTimeUs() const { return m_MaxTimeUs; }
  const MemoryTracker& GetMemoryTracker() const { return m_MemTracker; }
  const TimeGraphLayout& GetLayout() const { return m_Layout; }
  TimeGraphLayout& GetLayout() { return m_Layout; }

  void OnLeft();
  void OnRight();
  void OnUp();
  void OnDown();

  Color GetEventTrackColor(Timer timer);
  Color GetTimesliceColor(Timer timer);
  StringManager* GetStringManager() { return string_manager_.get(); }

 protected:
  void AddTrack(std::unique_ptr<Track> track);
  std::shared_ptr<ThreadTrack> GetOrCreateThreadTrack(ThreadID a_TID);

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
  std::map<ThreadID, int> m_ThreadDepths;
  std::map<ThreadID, uint32_t> m_EventCount;
  double m_TimeWindowUs = 0;
  float m_WorldStartX = 0;
  float m_WorldWidth = 0;
  float min_y_ = 0;
  int m_Margin = 0;

  double m_ZoomValue = 0;
  double m_MouseRatio = 0;
  unsigned int m_MainFrameCounter = 0;

  TimeGraphLayout m_Layout;

  std::map<DWORD /*ThreadId*/, std::map<long long, ContextSwitch>>
      m_ContextSwitchesMap;
  std::map<DWORD /*CoreId*/, std::map<long long, ContextSwitch>>
      m_CoreUtilizationMap;

  std::map<ThreadID, uint32_t> m_ThreadCountMap;

  bool m_NeedsUpdatePrimitives = false;
  bool m_DrawText = true;
  bool m_NeedsRedraw = false;
  Batcher m_Batcher;
  PickingManager* m_PickingManager = nullptr;
  Timer m_LastThreadReorder;
  MemoryTracker m_MemTracker;
  std::shared_ptr<Systrace> m_Systrace;

  mutable Mutex m_Mutex;
  std::vector<std::shared_ptr<Track>> tracks_;
  std::unordered_map<ThreadID, std::shared_ptr<ThreadTrack>> thread_tracks_;
  std::vector<std::shared_ptr<Track>> sorted_tracks_;
  std::string m_ThreadFilter;

  std::shared_ptr<StringManager> string_manager_;
};

extern TimeGraph* GCurrentTimeGraph;
