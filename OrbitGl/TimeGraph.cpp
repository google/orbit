//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TimeGraph.h"

#include <OrbitBase/Logging.h>

#include <algorithm>
#include <utility>

#include "App.h"
#include "Batcher.h"
#include "Capture.h"
#include "EventTracer.h"
#include "EventTrack.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "GraphTrack.h"
#include "Log.h"
#include "OrbitType.h"
#include "OrbitUnreal.h"
#include "Params.h"
#include "PickingManager.h"
#include "SamplingProfiler.h"
#include "SchedulerTrack.h"
#include "StringManager.h"
#include "Systrace.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "TimerManager.h"
#include "Utils.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

TimeGraph* GCurrentTimeGraph = nullptr;

namespace {

Color GetThreadColor(ThreadID a_TID) {
  static unsigned char a = 255;
  static std::vector<Color> s_ThreadColors{
      Color(231, 68, 53, a),    // red
      Color(43, 145, 175, a),   // blue
      Color(185, 117, 181, a),  // purple
      Color(87, 166, 74, a),    // green
      Color(215, 171, 105, a),  // beige
      Color(248, 101, 22, a)    // orange
  };
  return s_ThreadColors[a_TID % s_ThreadColors.size()];
}

}  // namespace

Color TimeGraph::GetEventTrackColor(Timer timer) {
  if (timer.m_Type == Timer::GPU_ACTIVITY) {
    const Color kGray(100, 100, 100, 255);
    return kGray;
  } else {
    return GetThreadColor(timer.m_TID);
  }
}

Color TimeGraph::GetTimesliceColor(Timer timer) {
  if (timer.m_Type == Timer::GPU_ACTIVITY) {
    // We color code the timeslices for GPU activity using the color
    // of the CPU thread track that submitted the job.
    Color col = GetThreadColor(timer.m_SubmitTID);

    // We disambiguate the different types of GPU activity based on the
    // string that is displayed on their timeslice.
    constexpr const char* kSwQueueString = "sw queue";
    constexpr const char* kHwQueueString = "hw queue";
    constexpr const char* kHwExecutionString = "hw execution";
    float coeff = 1.0f;
    std::string gpu_stage =
        string_manager_->Get(timer.m_UserData[0]).value_or("");
    if (gpu_stage == kSwQueueString) {
      coeff = 0.5f;
    } else if (gpu_stage == kHwQueueString) {
      coeff = 0.75f;
    } else if (gpu_stage == kHwExecutionString) {
      coeff = 1.0f;
    }

    col[0] = static_cast<uint8_t>(coeff * col[0]);
    col[1] = static_cast<uint8_t>(coeff * col[1]);
    col[2] = static_cast<uint8_t>(coeff * col[2]);

    return col;
  } else {
    return GetThreadColor(timer.m_TID);
  }
}

//-----------------------------------------------------------------------------
TimeGraph::TimeGraph() { m_LastThreadReorder.Start(); }

//-----------------------------------------------------------------------------
void TimeGraph::SetStringManager(std::shared_ptr<StringManager> str_manager) {
  string_manager_ = std::move(str_manager);
}

//-----------------------------------------------------------------------------
void TimeGraph::SetCanvas(GlCanvas* a_Canvas) {
  m_Canvas = a_Canvas;
  m_TextRenderer->SetCanvas(a_Canvas);
  m_TextRendererStatic.SetCanvas(a_Canvas);
}

//-----------------------------------------------------------------------------
void TimeGraph::SetFontSize(int a_FontSize) {
  m_TextRenderer->SetFontSize(a_FontSize);
  m_TextRendererStatic.SetFontSize(a_FontSize);
}

//-----------------------------------------------------------------------------
void TimeGraph::Clear() {
  m_Batcher.Reset();
  m_SessionMinCounter = 0xFFFFFFFFFFFFFFFF;
  m_SessionMaxCounter = 0;
  m_ThreadCountMap.clear();
  GEventTracer.GetEventBuffer().Reset();
  m_MemTracker.Clear();

  ScopeLock lock(m_Mutex);
  tracks_.clear();
  thread_tracks_.clear();

  m_ContextSwitchesMap.clear();
  m_CoreUtilizationMap.clear();
}

//-----------------------------------------------------------------------------
double GNumHistorySeconds = 2.f;

//-----------------------------------------------------------------------------
bool TimeGraph::UpdateSessionMinMaxCounter() {
  m_SessionMinCounter = LLONG_MAX;

  m_Mutex.lock();
  for (auto& track : tracks_) {
    if (track->GetNumTimers()) {
      TickType min = track->GetMinTime();
      if (min > 0 && min < m_SessionMinCounter) {
        m_SessionMinCounter = min;
      }
    }
  }
  m_Mutex.unlock();

  if (GEventTracer.GetEventBuffer().HasEvent()) {
    m_SessionMinCounter = std::min((long long)m_SessionMinCounter,
                                   GEventTracer.GetEventBuffer().GetMinTime());
    m_SessionMaxCounter = std::max((long long)m_SessionMaxCounter,
                                   GEventTracer.GetEventBuffer().GetMaxTime());
  }

  return m_SessionMinCounter != LLONG_MAX;
}

//-----------------------------------------------------------------------------
void TimeGraph::ZoomAll() {
  if (UpdateSessionMinMaxCounter()) {
    m_MaxTimeUs =
        MicroSecondsFromTicks(m_SessionMinCounter, m_SessionMaxCounter);
    m_MinTimeUs = m_MaxTimeUs - (GNumHistorySeconds * 1000 * 1000);
    if (m_MinTimeUs < 0) m_MinTimeUs = 0;

    NeedsUpdate();
    UpdatePrimitives();
  }
}

//-----------------------------------------------------------------------------
void TimeGraph::Zoom(const TextBox* a_TextBox) {
  const Timer& timer = a_TextBox->GetTimer();

  double start = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_Start);
  double end = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_End);

  double mid = start + ((end - start) / 2.0);
  double extent = 1.1 * (end - start) / 2.0;

  SetMinMax(mid - extent, mid + extent);
}

//-----------------------------------------------------------------------------
double TimeGraph::GetSessionTimeSpanUs() {
  if (UpdateSessionMinMaxCounter()) {
    return MicroSecondsFromTicks(m_SessionMinCounter, m_SessionMaxCounter);
  }

  return 0;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetCurrentTimeSpanUs() { return m_MaxTimeUs - m_MinTimeUs; }

//-----------------------------------------------------------------------------
void TimeGraph::ZoomTime(float a_ZoomValue, double a_MouseRatio) {
  m_ZoomValue = a_ZoomValue;
  m_MouseRatio = a_MouseRatio;

  static double incrementRatio = 0.1;
  double scale = a_ZoomValue > 0 ? 1 + incrementRatio : 1 - incrementRatio;

  double CurrentTimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_RefTimeUs = m_MinTimeUs + a_MouseRatio * CurrentTimeWindowUs;

  double timeLeft = std::max(m_RefTimeUs - m_MinTimeUs, 0.0);
  double timeRight = std::max(m_MaxTimeUs - m_RefTimeUs, 0.0);

  double minTimeUs = m_RefTimeUs - scale * timeLeft;
  double maxTimeUs = m_RefTimeUs + scale * timeRight;

  if (maxTimeUs - minTimeUs < 0.001 /*1 ns*/) {
    return;
  }

  SetMinMax(minTimeUs, maxTimeUs);
}

//-----------------------------------------------------------------------------
void TimeGraph::SetMinMax(double a_MinTimeUs, double a_MaxTimeUs) {
  double desiredTimeWindow = a_MaxTimeUs - a_MinTimeUs;
  m_MinTimeUs = std::max(a_MinTimeUs, 0.0);
  m_MaxTimeUs =
      std::min(m_MinTimeUs + desiredTimeWindow, GetSessionTimeSpanUs());

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::PanTime(int a_InitialX, int a_CurrentX, int a_Width,
                        double a_InitialTime) {
  m_TimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  double initialLocalTime =
      (double)a_InitialX / (double)a_Width * m_TimeWindowUs;
  double dt =
      (double)(a_CurrentX - a_InitialX) / (double)a_Width * m_TimeWindowUs;
  double currentTime = a_InitialTime - dt;
  m_MinTimeUs = clamp(currentTime - initialLocalTime, 0.0,
                      GetSessionTimeSpanUs() - m_TimeWindowUs);
  m_MaxTimeUs = m_MinTimeUs + m_TimeWindowUs;

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::OnDrag(float a_Ratio) {
  double timeSpan = GetSessionTimeSpanUs();
  double timeWindow = m_MaxTimeUs - m_MinTimeUs;
  m_MinTimeUs = a_Ratio * (timeSpan - timeWindow);
  m_MaxTimeUs = m_MinTimeUs + timeWindow;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetTime(double a_Ratio) {
  double CurrentWidth = m_MaxTimeUs - m_MinTimeUs;
  double Delta = a_Ratio * CurrentWidth;
  return m_MinTimeUs + Delta;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetTimeIntervalMicro(double a_Ratio) {
  double CurrentWidth = m_MaxTimeUs - m_MinTimeUs;
  return a_Ratio * CurrentWidth;
}

//-----------------------------------------------------------------------------
void TimeGraph::ProcessTimer(const Timer& a_Timer) {
  if (a_Timer.m_End > m_SessionMaxCounter) {
    m_SessionMaxCounter = a_Timer.m_End;
  }

  switch (a_Timer.m_Type) {
    case Timer::ALLOC:
      m_MemTracker.ProcessAlloc(a_Timer);
      return;
    case Timer::FREE:
      m_MemTracker.ProcessFree(a_Timer);
      return;
    case Timer::CORE_ACTIVITY:
      Capture::GHasContextSwitches = true;
      break;
    default:
      break;
  }

  if (a_Timer.m_FunctionAddress > 0) {
    Function* func = Capture::GTargetProcess->GetFunctionFromAddress(
        a_Timer.m_FunctionAddress);
    if (func != nullptr) {
      ++Capture::GFunctionCountMap[a_Timer.m_FunctionAddress];
      func->UpdateStats(a_Timer);
    }
  }

  std::shared_ptr<ThreadTrack> track = GetOrCreateThreadTrack(a_Timer.m_TID);
  if (a_Timer.m_Type == Timer::GPU_ACTIVITY) {
    track->SetName(string_manager_->Get(a_Timer.m_UserData[1]).value_or(""));
    track->SetLabelDisplayMode(Track::NAME_ONLY);
    track->SetEventTrackColor(GetEventTrackColor(a_Timer));
  }
  if (a_Timer.m_Type == Timer::INTROSPECTION) {
    const Color kGreenIntrospection(87, 166, 74, 255);
    track->SetColor(kGreenIntrospection);
  }

  if (!a_Timer.IsType(Timer::THREAD_ACTIVITY) &&
      !a_Timer.IsType(Timer::CORE_ACTIVITY)) {
    track->OnTimer(a_Timer);
    ++m_ThreadCountMap[a_Timer.m_TID];
  } else {
    // Use thead 0 as container for scheduling events.  TODO: most of this
    // should be done once.
    const std::shared_ptr<ThreadTrack>& track0 = GetOrCreateThreadTrack(0);
    std::string process_name = Capture::GTargetProcess->GetName();
    track0->SetName(process_name + " (all threads)");
    track0->SetLabelDisplayMode(Track::NAME_ONLY);
    track0->SetEventTrackColor(GetThreadColor(0));
    track0->OnTimer(a_Timer);
    ++m_ThreadCountMap[0];
  }
}

//-----------------------------------------------------------------------------
uint32_t TimeGraph::GetNumTimers() const {
  uint32_t numTimers = 0;
  ScopeLock lock(m_Mutex);
  for (const auto& track : tracks_) {
    numTimers += track->GetNumTimers();
  }
  return numTimers;
}

//-----------------------------------------------------------------------------
uint32_t TimeGraph::GetNumCores() const {
  ScopeLock lock(m_Mutex);
  return m_CoreUtilizationMap.size();
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> TimeGraph::GetAllTimerChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& track : tracks_) {
    Append(chains, track->GetAllChains());
  }
  return chains;
}

//-----------------------------------------------------------------------------
void TimeGraph::AddContextSwitch(const ContextSwitch& a_CS) {
  auto pair = std::make_pair(a_CS.m_Time, a_CS);

  if (a_CS.m_Type == ContextSwitch::Out) {
    if (true) {
      // Processor time line
      std::map<long long, ContextSwitch>& csMap =
          m_CoreUtilizationMap[a_CS.m_ProcessorIndex];

      if (!csMap.empty()) {
        ContextSwitch& lastCS = csMap.rbegin()->second;
        if (lastCS.m_Type == ContextSwitch::In) {
          Timer timer;
          timer.m_Start = lastCS.m_Time;
          timer.m_End = a_CS.m_Time;
          // When a context switch out is caused by a thread exiting, the
          // perf_event_open event has pid and tid set to -1: hence, use pid and
          // tid from the context switch in.
          timer.m_PID = lastCS.m_ProcessId;
          timer.m_TID = lastCS.m_ThreadId;
          timer.m_Processor = static_cast<int8_t>(lastCS.m_ProcessorIndex);
          timer.m_SessionID = Message::GSessionID;
          timer.SetType(Timer::CORE_ACTIVITY);

          GTimerManager->Add(timer);
        }
      }
    }

    if (false) {
      // Thread time line
      std::map<long long, ContextSwitch>& csMap =
          m_ContextSwitchesMap[a_CS.m_ThreadId];

      if (!csMap.empty()) {
        ContextSwitch& lastCS = csMap.rbegin()->second;
        if (lastCS.m_Type == ContextSwitch::In) {
          Timer timer;
          timer.m_Start = lastCS.m_Time;
          timer.m_End = a_CS.m_Time;
          // When a context switch out is caused by a thread exiting, the
          // perf_event_open event has pid and tid set to -1: hence, use pid and
          // tid from the context switch in.
          timer.m_PID = lastCS.m_ProcessId;
          timer.m_TID = lastCS.m_ThreadId;
          timer.m_SessionID = Message::GSessionID;
          timer.SetType(Timer::THREAD_ACTIVITY);

          GTimerManager->Add(timer);
        }
      }
    }
  }

  // TODO: if events are already sorted by timestamp, then
  //       we don't need to use maps. To investigate...
  m_ContextSwitchesMap[a_CS.m_ThreadId].insert(pair);
  m_CoreUtilizationMap[a_CS.m_ProcessorIndex].insert(pair);
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateMaxTimeStamp(TickType a_Time) {
  if (a_Time > m_SessionMaxCounter) {
    m_SessionMaxCounter = a_Time;
  }
};

//-----------------------------------------------------------------------------
void TimeGraph::UpdateThreadDepth(int a_ThreadId, int a_Depth) {
  if (a_Depth > m_ThreadDepths[a_ThreadId]) {
    m_ThreadDepths[a_ThreadId] = a_Depth;
  }
}

//-----------------------------------------------------------------------------
float TimeGraph::GetThreadTotalHeight() { return std::abs(min_y_); }

//-----------------------------------------------------------------------------
float TimeGraph::GetWorldFromTick(TickType a_Time) const {
  if (m_TimeWindowUs > 0) {
    double start =
        MicroSecondsFromTicks(m_SessionMinCounter, a_Time) - m_MinTimeUs;
    double normalizedStart = start / m_TimeWindowUs;
    float pos = float(m_WorldStartX + normalizedStart * m_WorldWidth);
    return pos;
  }

  return 0;
}

//-----------------------------------------------------------------------------
float TimeGraph::GetWorldFromUs(double a_Micros) const {
  return GetWorldFromTick(GetTickFromUs(a_Micros));
}

//-----------------------------------------------------------------------------
double TimeGraph::GetUsFromTick(TickType time) const {
  return MicroSecondsFromTicks(m_SessionMinCounter, time) - m_MinTimeUs;
}

//-----------------------------------------------------------------------------
TickType TimeGraph::GetTickFromWorld(float a_WorldX) {
  double ratio =
      m_WorldWidth != 0
          ? static_cast<double>((a_WorldX - m_WorldStartX) / m_WorldWidth)
          : 0;
  double timeStamp = GetTime(ratio);

  return m_SessionMinCounter + TicksFromMicroseconds(timeStamp);
}

//-----------------------------------------------------------------------------
TickType TimeGraph::GetTickFromUs(double a_MicroSeconds) const {
  return m_SessionMinCounter + TicksFromMicroseconds(a_MicroSeconds);
}

//-----------------------------------------------------------------------------
void TimeGraph::GetWorldMinMax(float& a_Min, float& a_Max) const {
  a_Min = GetWorldFromTick(m_SessionMinCounter);
  a_Max = GetWorldFromTick(m_SessionMaxCounter);
}

//-----------------------------------------------------------------------------
void TimeGraph::SelectLeft(const TextBox* a_TextBox) {
  TextBox* textBox = const_cast<TextBox*>(a_TextBox);
  Capture::GSelectedTextBox = textBox;
  const Timer& timer = textBox->GetTimer();

  if (IsVisible(timer)) {
    return;
  }

  double currentTimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_RefTimeUs = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_Start);

  double minTimeUs = m_RefTimeUs;
  double maxTimeUs = m_RefTimeUs + currentTimeWindowUs;

  SetMinMax(minTimeUs, maxTimeUs);
}

//-----------------------------------------------------------------------------
void TimeGraph::SelectRight(const TextBox* a_TextBox) {
  TextBox* textBox = const_cast<TextBox*>(a_TextBox);
  Capture::GSelectedTextBox = textBox;
  const Timer& timer = textBox->GetTimer();

  if (IsVisible(timer)) {
    return;
  }

  double currentTimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_RefTimeUs = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_End);

  static double ratio = 1.0;
  double minTimeUs = m_RefTimeUs - ratio * currentTimeWindowUs;
  double maxTimeUs = m_RefTimeUs + (1 - ratio) * currentTimeWindowUs;

  SetMinMax(minTimeUs, maxTimeUs);
}

//-----------------------------------------------------------------------------
void TimeGraph::NeedsUpdate() { m_NeedsUpdatePrimitives = true; }

//-----------------------------------------------------------------------------
void TimeGraph::UpdatePrimitives() {
  CHECK(string_manager_);

  m_Batcher.Reset();
  m_TextRendererStatic.Clear();

  UpdateMaxTimeStamp(GEventTracer.GetEventBuffer().GetMaxTime());

  m_SceneBox = m_Canvas->GetSceneBox();
  m_TimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_WorldStartX = m_Canvas->GetWorldTopLeftX();
  m_WorldWidth = m_Canvas->GetWorldWidth();
  uint64_t min_tick = GetTickFromUs(m_MinTimeUs);
  uint64_t max_tick = GetTickFromUs(m_MaxTimeUs);

  SortTracks();

  float current_y = 0.f;

  for (auto& track : sorted_tracks_) {
    track->SetY(current_y);
    track->UpdatePrimitives(min_tick, max_tick);
    current_y -= (track->GetHeight() + m_Layout.GetSpaceBetweenTracks());
  }

  min_y_ = current_y;
  m_NeedsUpdatePrimitives = false;
  m_NeedsRedraw = true;
}

//-----------------------------------------------------------------------------
std::vector<CallstackEvent> TimeGraph::SelectEvents(float a_WorldStart,
                                                    float a_WorldEnd,
                                                    ThreadID a_TID) {
  if (a_WorldStart > a_WorldEnd) {
    std::swap(a_WorldEnd, a_WorldStart);
  }

  TickType t0 = GetTickFromWorld(a_WorldStart);
  TickType t1 = GetTickFromWorld(a_WorldEnd);

  std::vector<CallstackEvent> selected_callstack_events =
      GEventTracer.GetEventBuffer().GetCallstackEvents(t0, t1, a_TID);

  // Generate report
  std::shared_ptr<SamplingProfiler> samplingProfiler =
      std::make_shared<SamplingProfiler>(Capture::GTargetProcess);

  samplingProfiler->SetIsLinuxPerf(
      Capture::IsRemote());  // TODO: could be windows->windows remote capture
  samplingProfiler->SetState(SamplingProfiler::Sampling);
  samplingProfiler->SetGenerateSummary(a_TID == 0);

  for (CallstackEvent& event : selected_callstack_events) {
    std::shared_ptr<CallStack> callstack =
        Capture::GSamplingProfiler->GetCallStack(event.m_Id);
    if (callstack) {
      callstack->m_ThreadId = event.m_TID;
      samplingProfiler->AddCallStack(*callstack);
    }
  }
  samplingProfiler->ProcessSamples();

  if (samplingProfiler->GetNumSamples() > 0) {
    GOrbitApp->AddSelectionReport(samplingProfiler);
  }

  NeedsUpdate();

  return selected_callstack_events;
}

//-----------------------------------------------------------------------------
void TimeGraph::Draw(bool a_Picking) {
  if ((!a_Picking && m_NeedsUpdatePrimitives) || a_Picking) {
    UpdatePrimitives();
  }

  DrawTracks(a_Picking);
  DrawBuffered(a_Picking);

  m_NeedsRedraw = false;
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawTracks(bool a_Picking) {
  m_Layout.SetNumCores(GetNumCores());
  for (auto& track : sorted_tracks_) {
    if (track->GetName().empty()) {
      if (track->GetType() == Track::kThreadTrack) {
        std::string threadName =
            Capture::GTargetProcess->GetThreadNameFromTID(track->GetID());
        track->SetName(threadName);
      }
    }

    track->Draw(m_Canvas, a_Picking);
  }
}

//-----------------------------------------------------------------------------
std::shared_ptr<ThreadTrack> TimeGraph::GetOrCreateThreadTrack(ThreadID a_TID) {
  ScopeLock lock(m_Mutex);
  std::shared_ptr<ThreadTrack> track = thread_tracks_[a_TID];
  if (track == nullptr) {
    if (a_TID == 0) {
      track = std::make_shared<SchedulerTrack>(this, a_TID);
    } else {
      track = std::make_shared<ThreadTrack>(this, a_TID);
    }
    tracks_.emplace_back(track);
    thread_tracks_[a_TID] = track;
    track->SetEventTrackColor(GetThreadColor(a_TID));
  }
  return track;
}

//-----------------------------------------------------------------------------
void TimeGraph::SetThreadFilter(const std::string& a_Filter) {
  std::cout << "Setting thread filter: " << a_Filter << std::endl;
  m_ThreadFilter = a_Filter;
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::SortTracks() {
  // Get or create thread track from events' thread id.
  {
    ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
    m_EventCount.clear();

    for (auto& pair : GEventTracer.GetEventBuffer().GetCallstacks()) {
      ThreadID threadID = pair.first;
      std::map<uint64_t, CallstackEvent>& callstacks = pair.second;
      m_EventCount[threadID] = (uint32_t)callstacks.size();
      GetOrCreateThreadTrack(threadID);
    }
  }

  // Reorder threads once every second when capturing
  if (!Capture::IsCapturing() || m_LastThreadReorder.QueryMillis() > 1000.0) {
    sorted_tracks_.clear();

    // Sched track is currently held as thread 0, TODO: make it it's own track.
    sorted_tracks_.emplace_back(GetOrCreateThreadTrack(0));

    std::vector<ThreadID> sortedThreadIds;

    // Thread "0" holds scheduling information and is used to select callstacks
    // from all threads, show it at the top.
    sortedThreadIds.push_back(0);

    // Show threads with instrumented functions first
    std::vector<std::pair<ThreadID, uint32_t>> sortedThreads =
        OrbitUtils::ReverseValueSort(m_ThreadCountMap);
    for (auto& pair : sortedThreads) {
      // Scheduling information is held in thread "0", which is handled
      // separately.
      // TODO: Make a proper "SchedTrack" instead of a hack.
      if (pair.first != 0) sortedThreadIds.push_back(pair.first);
    }

    // Then show threads sorted by number of events
    std::vector<std::pair<ThreadID, uint32_t>> sortedByEvents =
        OrbitUtils::ReverseValueSort(m_EventCount);
    for (auto& pair : sortedByEvents) {
      if (m_ThreadCountMap.find(pair.first) == m_ThreadCountMap.end()) {
        sortedThreadIds.push_back(pair.first);
      }
    }

    // Filter thread ids if needed
    if (!m_ThreadFilter.empty()) {
      std::vector<std::string> filters = Tokenize(m_ThreadFilter, " ");
      std::vector<ThreadID> filteredThreadIds;
      for (ThreadID tid : sortedThreadIds) {
        std::shared_ptr<ThreadTrack> track = GetOrCreateThreadTrack(tid);

        for (auto& filter : filters) {
          if (track && absl::StrContains(track->GetName(), filter)) {
            filteredThreadIds.push_back(tid);
          }
        }
      }
      sortedThreadIds = filteredThreadIds;
    }

    // Thread Tracks.
    for (auto thread_id : sortedThreadIds) {
      sorted_tracks_.emplace_back(GetOrCreateThreadTrack(thread_id));
    }

    m_LastThreadReorder.Reset();
  }
}

//----------------------------------------------------------------------------
void TimeGraph::OnLeft() {
  TextBox* selection = Capture::GSelectedTextBox;
  if (selection) {
    const Timer& timer = selection->GetTimer();
    const TextBox* left =
        GetOrCreateThreadTrack(timer.m_TID)->GetLeft(selection);
    if (left) {
      SelectLeft(left);
    }
  }
  NeedsUpdate();
}

//----------------------------------------------------------------------------
void TimeGraph::OnRight() {
  TextBox* selection = Capture::GSelectedTextBox;
  if (selection) {
    const Timer& timer = selection->GetTimer();
    const TextBox* right =
        GetOrCreateThreadTrack(timer.m_TID)->GetRight(selection);
    if (right) {
      SelectRight(right);
    }
  }
  NeedsUpdate();
}

//----------------------------------------------------------------------------
void TimeGraph::OnUp() {
  TextBox* selection = Capture::GSelectedTextBox;
  if (selection) {
    const Timer& timer = selection->GetTimer();
    const TextBox* up = GetOrCreateThreadTrack(timer.m_TID)->GetUp(selection);
    if (up) {
      Select(up);
    }
  }
  NeedsUpdate();
}

//----------------------------------------------------------------------------
void TimeGraph::OnDown() {
  TextBox* selection = Capture::GSelectedTextBox;
  if (selection) {
    const Timer& timer = selection->GetTimer();
    const TextBox* down =
        GetOrCreateThreadTrack(timer.m_TID)->GetDown(selection);
    if (down) {
      Select(down);
    }
  }
  NeedsUpdate();
}

//----------------------------------------------------------------------------
void TimeGraph::DrawText() {
  if (m_DrawText) {
    m_TextRendererStatic.Display();
  }
}

//----------------------------------------------------------------------------
void TimeGraph::DrawBuffered(bool a_Picking) {
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);

  DrawBoxBuffer(a_Picking);
  DrawLineBuffer(a_Picking);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopAttrib();
}

//----------------------------------------------------------------------------
void TimeGraph::DrawBoxBuffer(bool a_Picking) {
  Block<Box, BoxBuffer::NUM_BOXES_PER_BLOCK>* boxBlock =
      m_Batcher.GetBoxBuffer().m_Boxes.m_Root;
  Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK * 4>* colorBlock;

  colorBlock = !a_Picking ? m_Batcher.GetBoxBuffer().m_Colors.m_Root
                          : m_Batcher.GetBoxBuffer().m_PickingColors.m_Root;

  while (boxBlock) {
    if (int numElems = boxBlock->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), boxBlock->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color),
                     (void*)(colorBlock->m_Data));
      glDrawArrays(GL_QUADS, 0, numElems * 4);
    }

    boxBlock = boxBlock->m_Next;
    colorBlock = colorBlock->m_Next;
  }
}

//----------------------------------------------------------------------------
void TimeGraph::DrawLineBuffer(bool a_Picking) {
  Block<Line, LineBuffer::NUM_LINES_PER_BLOCK>* lineBlock =
      m_Batcher.GetLineBuffer().m_Lines.m_Root;
  Block<Color, LineBuffer::NUM_LINES_PER_BLOCK * 2>* colorBlock;

  colorBlock = !a_Picking ? m_Batcher.GetLineBuffer().m_Colors.m_Root
                          : m_Batcher.GetLineBuffer().m_PickingColors.m_Root;

  while (lineBlock) {
    if (int numElems = lineBlock->m_Size) {
      glVertexPointer(3, GL_FLOAT, sizeof(Vec3), lineBlock->m_Data);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color),
                     (void*)(colorBlock->m_Data));
      glDrawArrays(GL_LINES, 0, numElems * 2);
    }

    lineBlock = lineBlock->m_Next;
    colorBlock = colorBlock->m_Next;
  }
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawMainFrame(TextBox& a_Box) {
  if (a_Box.GetMainFrameCounter() == -1) {
    a_Box.SetMainFrameCounter(++m_MainFrameCounter);
  }

  static unsigned char grey = 180;
  static Color frameColor(grey, grey, grey, 10);

  if (a_Box.GetMainFrameCounter() % 2 == 0) {
    float minX = m_SceneBox.GetPosX();
    TextBox frameBox;

    frameBox.SetPosX(a_Box.GetPosX());
    frameBox.SetPosY(m_SceneBox.GetPosY());
    frameBox.SetSizeX(a_Box.GetSize()[0]);
    frameBox.SetSizeY(m_SceneBox.GetSize()[0]);
    frameBox.SetColor(frameColor);
    frameBox.Draw(*m_TextRenderer, minX, true, false, false);
  }
}

//-----------------------------------------------------------------------------
bool TimeGraph::IsVisible(const Timer& a_Timer) {
  double start = MicroSecondsFromTicks(m_SessionMinCounter, a_Timer.m_Start);
  double end = MicroSecondsFromTicks(m_SessionMinCounter, a_Timer.m_End);

  double startUs = m_MinTimeUs;

  if (startUs > end || m_MaxTimeUs < start) {
    return false;
  }

  return true;
}
