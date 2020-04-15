#include "ThreadTrack.h"

#include <limits>

#include "EventTrack.h"
#include "GlCanvas.h"
#include "TimeGraph.h"

//-----------------------------------------------------------------------------
ThreadTrack::ThreadTrack(TimeGraph* a_TimeGraph, uint32_t a_ThreadID) {
  m_TimeGraph = a_TimeGraph;
  m_ID = a_ThreadID;
  m_TextRenderer = a_TimeGraph->GetTextRenderer();
  m_ThreadID = a_ThreadID;

  m_NumTimers = 0;
  m_MinTime = std::numeric_limits<TickType>::max();
  m_MaxTime = std::numeric_limits<TickType>::min();

  m_EventTrack = std::make_shared<EventTrack>(a_TimeGraph);
  m_EventTrack->SetThreadId(a_ThreadID);
}

//-----------------------------------------------------------------------------
void ThreadTrack::Draw(GlCanvas* a_Canvas, bool a_Picking) {
  // Scheduling information is held in thread "0", don't draw as threadtrack.
  // TODO: Make a proper "SchedTrack" instead of hack.
  if (m_ID == 0) return;

  TimeGraphLayout& layout = m_TimeGraph->GetLayout();
  float threadOffset = layout.GetThreadBlockStart(m_ID);
  float trackHeight = GetHeight();
  float trackWidth = a_Canvas->GetWorldWidth();

  SetPos(a_Canvas->GetWorldTopLeftX(), threadOffset);
  SetSize(trackWidth, trackHeight);

  Track::Draw(a_Canvas, a_Picking);

  // Event track
  m_EventTrack->SetPos(m_Pos[0], m_Pos[1]);
  m_EventTrack->SetSize(a_Canvas->GetWorldWidth(),
                        m_TimeGraph->GetLayout().GetEventTrackHeight());
  m_EventTrack->Draw(a_Canvas, a_Picking);
}

//-----------------------------------------------------------------------------
void ThreadTrack::OnDrag(int a_X, int a_Y) { Track::OnDrag(a_X, a_Y); }

//-----------------------------------------------------------------------------
void ThreadTrack::OnTimer(const Timer& a_Timer) {
  UpdateDepth(a_Timer.m_Depth + 1);
  TextBox textBox(Vec2(0, 0), Vec2(0, 0), "", Color(255, 0, 0, 255));
  textBox.SetTimer(a_Timer);

  std::shared_ptr<TimerChain> timerChain = m_Timers[a_Timer.m_Depth];
  if (timerChain == nullptr) {
    timerChain = std::make_shared<TimerChain>();
    m_Timers[a_Timer.m_Depth] = timerChain;
  }
  timerChain->push_back(textBox);
  ++m_NumTimers;
  if (a_Timer.m_Start < m_MinTime) m_MinTime = a_Timer.m_Start;
  if (a_Timer.m_End > m_MaxTime) m_MaxTime = a_Timer.m_End;
}

//-----------------------------------------------------------------------------
float ThreadTrack::GetHeight() const {
  TimeGraphLayout& layout = m_TimeGraph->GetLayout();
  return layout.GetTextBoxHeight() * GetDepth() +
         layout.GetSpaceBetweenTracksAndThread() +
         layout.GetEventTrackHeight() + layout.GetTrackBottomMargin();
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> ThreadTrack::GetTimers() {
  std::vector<std::shared_ptr<TimerChain>> timers;
  ScopeLock lock(m_Mutex);
  for (auto& timerChain : m_Timers) {
    timers.push_back(timerChain.second);
  }
  return timers;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetFirstAfterTime(TickType a_Tick,
                                              uint32_t a_Depth) const {
  std::shared_ptr<TimerChain> textBoxes = GetTimers(a_Depth);
  if (textBoxes == nullptr) return nullptr;

  // TODO: do better than linear search...
  for (TextBox& textBox : *textBoxes) {
    if (textBox.GetTimer().m_Start > a_Tick) {
      return &textBox;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetFirstBeforeTime(TickType a_Tick,
                                               uint32_t a_Depth) const {
  std::shared_ptr<TimerChain> textBoxes = GetTimers(a_Depth);
  if (textBoxes == nullptr) return nullptr;

  TextBox* textBox = nullptr;

  // TODO: do better than linear search...
  for (TextBox& box : *textBoxes) {
    if (box.GetTimer().m_Start > a_Tick) {
      return textBox;
    }

    textBox = &box;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<TimerChain> ThreadTrack::GetTimers(uint32_t a_Depth) const {
  ScopeLock lock(m_Mutex);
  auto it = m_Timers.find(a_Depth);
  if (it != m_Timers.end()) return it->second;
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetLeft(TextBox* a_TextBox) const {
  const Timer& timer = a_TextBox->GetTimer();
  if (timer.m_TID == m_ThreadID) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer.m_Depth);
    if (timers) return timers->GetElementBefore(a_TextBox);
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetRight(TextBox* a_TextBox) const {
  const Timer& timer = a_TextBox->GetTimer();
  if (timer.m_TID == m_ThreadID) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer.m_Depth);
    if (timers) return timers->GetElementAfter(a_TextBox);
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetUp(TextBox* a_TextBox) const {
  const Timer& timer = a_TextBox->GetTimer();
  return GetFirstBeforeTime(timer.m_Start, timer.m_Depth - 1);
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetDown(TextBox* a_TextBox) const {
  const Timer& timer = a_TextBox->GetTimer();
  return GetFirstAfterTime(timer.m_Start, timer.m_Depth + 1);
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> ThreadTrack::GetAllChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& pair : m_Timers) {
    chains.push_back(pair.second);
  }
  return chains;
}

//-----------------------------------------------------------------------------
void ThreadTrack::SetEventTrackColor(Color color) {
  m_EventTrack->SetColor(color);
}
