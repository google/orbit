//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TimeGraph.h"

#include <algorithm>
#include <OrbitBase/Logging.h>

#include "App.h"
#include "Batcher.h"
#include "Capture.h"
#include "EventTracer.h"
#include "EventTrack.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "Log.h"
#include "OrbitBase/Logging.h"
#include "OrbitType.h"
#include "OrbitUnreal.h"
#include "Params.h"
#include "Pdb.h"
#include "PickingManager.h"
#include "SamplingProfiler.h"
#include "StringManager.h"
#include "Systrace.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "TimerManager.h"
#include "Utils.h"
#include "absl/strings/str_format.h"

TimeGraph* GCurrentTimeGraph = nullptr;

//-----------------------------------------------------------------------------
TimeGraph::TimeGraph() { m_LastThreadReorder.Start(); }

//-----------------------------------------------------------------------------
void TimeGraph::SetStringManager(std::shared_ptr<StringManager> str_manager) {
  string_manager_ = str_manager;
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
  m_Layout.Reset();

  ScopeLock lock(m_Mutex);
  m_ThreadTracks.clear();

  m_ContextSwitchesMap.clear();
  m_CoreUtilizationMap.clear();
}

//-----------------------------------------------------------------------------
double GNumHistorySeconds = 2.f;

//-----------------------------------------------------------------------------
bool TimeGraph::UpdateSessionMinMaxCounter() {
  m_SessionMinCounter = LLONG_MAX;

  m_Mutex.lock();
  for (auto& pair : m_ThreadTracks) {
    auto& track = pair.second;
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
    UpdatePrimitives(false);
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

  if (!a_Timer.IsType(Timer::THREAD_ACTIVITY) &&
      !a_Timer.IsType(Timer::CORE_ACTIVITY)) {
    std::shared_ptr<ThreadTrack> track = GetThreadTrack(a_Timer.m_TID);
    if (a_Timer.m_Type == Timer::GPU_ACTIVITY) {
      track->SetName(string_manager_->Get(a_Timer.m_UserData[1]).value_or(""));
      track->SetLabelDisplayMode(Track::NAME_ONLY);
    }

    track->OnTimer(a_Timer);
    ++m_ThreadCountMap[a_Timer.m_TID];
    if (a_Timer.m_Type == Timer::INTROSPECTION) {
      const Color kGreenIntrospection(87, 166, 74, 255);
      track->SetColor(kGreenIntrospection);
    }
  } else {
    // Use thead 0 as container for scheduling events.
    GetThreadTrack(0)->OnTimer(a_Timer);
    ++m_ThreadCountMap[0];
  }
}

//-----------------------------------------------------------------------------
uint32_t TimeGraph::GetNumTimers() const {
  uint32_t numTimers = 0;
  ScopeLock lock(m_Mutex);
  for (auto& pair : m_ThreadTracks) {
    auto& track = pair.second;
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
  for (const auto& pair : m_ThreadTracks) {
    const std::shared_ptr<ThreadTrack>& track = pair.second;
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
          timer.m_TID = a_CS.m_ThreadId;
          timer.m_Processor = (int8_t)a_CS.m_ProcessorIndex;
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
          timer.m_TID = a_CS.m_ThreadId;
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
float TimeGraph::GetThreadTotalHeight() { return m_Layout.GetTotalHeight(); }

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
void TimeGraph::Select(const Vec2& a_WorldStart, const Vec2 a_WorldStop) {
  float x0 = std::min(a_WorldStart[0], a_WorldStop[0]);
  float x1 = std::max(a_WorldStart[0], a_WorldStop[0]);

  TickType t0 = GetTickFromWorld(x0);
  TickType t1 = GetTickFromWorld(x1);

  m_SelectedCallstackEvents =
      GEventTracer.GetEventBuffer().GetCallstackEvents(t0, t1);

  NeedsUpdate();
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

  double ratio = m_MarginRatio;
  double minTimeUs = m_RefTimeUs - ratio * currentTimeWindowUs;
  double maxTimeUs = m_RefTimeUs + (1 - ratio) * currentTimeWindowUs;

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
inline std::string GetExtraInfo(const Timer& a_Timer) {
  std::string info;
  if (!Capture::IsCapturing() && a_Timer.GetType() == Timer::UNREAL_OBJECT) {
    info =
        "[" + ws2s(GOrbitUnreal.GetObjectNames()[a_Timer.m_UserData[0]]) + "]";
  }
  return info;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdatePrimitives(bool a_Picking) {
  CHECK(string_manager_);

  m_Batcher.Reset();
  m_VisibleTextBoxes.clear();
  m_TextRendererStatic.Clear();
  m_TextRendererStatic.Init();  // TODO: needed?

  UpdateMaxTimeStamp(GEventTracer.GetEventBuffer().GetMaxTime());

  m_SceneBox = m_Canvas->GetSceneBox();
  float minX = m_SceneBox.GetPosX();
  m_NumDrawnTextBoxes = 0;

  m_TimeWindowUs = m_MaxTimeUs - m_MinTimeUs;
  m_WorldStartX = m_Canvas->GetWorldTopLeftX();
  m_WorldWidth = m_Canvas->GetWorldWidth();
  double invTimeWindow = 1.0 / m_TimeWindowUs;

  UpdateThreadIds();

  double span = m_MaxTimeUs - m_MinTimeUs;
  TickType rawStart = GetTickFromUs(m_MinTimeUs + m_MarginRatio * span);
  TickType rawStop = GetTickFromUs(m_MaxTimeUs);

  unsigned int TextBoxID = 0;

  for (auto& pair : GetThreadTracksCopy()) {
    std::shared_ptr<ThreadTrack>& threadTrack = pair.second;

    if (!m_Layout.IsThreadVisible(threadTrack->GetID())) continue;

    std::vector<std::shared_ptr<TimerChain>> depthChain =
        threadTrack->GetTimers();
    for (auto& textBoxes : depthChain) {
      if (textBoxes == nullptr) break;

      for (TextBox& textBox : *textBoxes) {
        const Timer& timer = textBox.GetTimer();

        if (!(rawStart > timer.m_End || rawStop < timer.m_Start)) {
          double start =
              MicroSecondsFromTicks(m_SessionMinCounter, timer.m_Start) -
              m_MinTimeUs;
          double end = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_End) -
                       m_MinTimeUs;
          double elapsed = end - start;

          double NormalizedStart = start * invTimeWindow;
          double NormalizedLength = elapsed * invTimeWindow;

          bool isCore = timer.IsType(Timer::CORE_ACTIVITY);

          float threadOffset =
              !isCore ? m_Layout.GetThreadOffset(timer.m_TID, timer.m_Depth)
                      : m_Layout.GetCoreOffset(timer.m_Processor);

          float boxHeight = !isCore ? m_Layout.GetTextBoxHeight()
                                    : m_Layout.GetTextCoresHeight();

          float WorldTimerStartX =
              float(m_WorldStartX + NormalizedStart * m_WorldWidth);
          float WorldTimerWidth = float(NormalizedLength * m_WorldWidth);

          Vec2 pos(WorldTimerStartX, threadOffset);
          Vec2 size(WorldTimerWidth, boxHeight);

          textBox.SetPos(pos);
          textBox.SetSize(size);

          if (!isCore) {
            UpdateThreadDepth(timer.m_TID, timer.m_Depth + 1);
          }

          bool isContextSwitch = timer.IsType(Timer::THREAD_ACTIVITY);
          bool isVisibleWidth = NormalizedLength * m_Canvas->getWidth() > 1;
          bool isSameThreadIdAsSelected =
              isCore && (timer.m_TID == Capture::GSelectedThreadId);
          bool isInactive =
              (!isContextSwitch && timer.m_FunctionAddress &&
               (Capture::GVisibleFunctionsMap.size() &&
                Capture::GVisibleFunctionsMap[timer.m_FunctionAddress] ==
                    nullptr)) ||
              (Capture::GSelectedThreadId != 0 && isCore &&
               !isSameThreadIdAsSelected);
          bool isSelected = &textBox == Capture::GSelectedTextBox;

          const unsigned char g = 100;
          Color grey(g, g, g, 255);
          static Color selectionColor(0, 128, 255, 255);
          Color col = GetThreadColor(timer.m_TID);

          // We disambiguate the different types of GPU activity based on the
          // string that is displayed on their timeslice.
          if (timer.m_Type == Timer::GPU_ACTIVITY) {
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

            col[0] = coeff * col[0];
            col[1] = coeff * col[1];
            col[2] = coeff * col[2];
          }

          col = isSelected
                    ? selectionColor
                    : isSameThreadIdAsSelected ? col : isInactive ? grey : col;
          textBox.SetColor(col[0], col[1], col[2]);
          static int oddAlpha = 210;
          if (!(timer.m_Depth & 0x1)) {
            col[3] = oddAlpha;
          }

          float z = isInactive ? GlCanvas::Z_VALUE_BOX_INACTIVE
                               : GlCanvas::Z_VALUE_BOX_ACTIVE;

          if (isVisibleWidth) {
            Box box;
            box.m_Vertices[0] = Vec3(pos[0], pos[1], z);
            box.m_Vertices[1] = Vec3(pos[0], pos[1] + size[1], z);
            box.m_Vertices[2] = Vec3(pos[0] + size[0], pos[1] + size[1], z);
            box.m_Vertices[3] = Vec3(pos[0] + size[0], pos[1], z);
            Color colors[4];
            Fill(colors, col);

            static float coeff = 0.94f;
            Vec3 dark = Vec3(col[0], col[1], col[2]) * coeff;
            colors[1] = Color((unsigned char)dark[0], (unsigned char)dark[1],
                              (unsigned char)dark[2], (unsigned char)col[3]);
            colors[0] = colors[1];
            m_Batcher.AddBox(box, colors, PickingID::BOX, &textBox);

            if (!isContextSwitch && textBox.GetText().size() == 0) {
              double elapsedMillis = ((double)elapsed) * 0.001;
              std::string time = GetPrettyTime(elapsedMillis);
              Function* func =
                  Capture::GSelectedFunctionsMap[timer.m_FunctionAddress];

              textBox.SetElapsedTimeTextLength(time.length());

              const char* name = nullptr;
              if (func) {
                std::string extraInfo = GetExtraInfo(timer);
                name = func->PrettyName().c_str();
                std::string text = absl::StrFormat(
                    "%s %s %s", name, extraInfo.c_str(), time.c_str());

                textBox.SetText(text);
              } else if (timer.m_Type == Timer::INTROSPECTION) {
                textBox.SetText(
                    string_manager_->Get(timer.m_UserData[0]).value_or(""));
              } else if (timer.m_Type == Timer::GPU_ACTIVITY) {
                textBox.SetText(
                    string_manager_->Get(timer.m_UserData[0]).value_or(""));
              } else if (!SystraceManager::Get().IsEmpty()) {
                textBox.SetText(SystraceManager::Get().GetFunctionName(
                    timer.m_FunctionAddress));
              } else if (!Capture::IsCapturing()) {
                // GZoneNames is populated when capturing, prevent race
                // by accessing it only when not capturing.
                auto it = Capture::GZoneNames.find(timer.m_FunctionAddress);
                if (it != Capture::GZoneNames.end()) {
                  name = it->second.c_str();
                  std::string text =
                      absl::StrFormat("%s %s", name, time.c_str());
                  textBox.SetText(text);
                }
              }
            }

            if (!isCore) {
              // m_VisibleTextBoxes.push_back(&textBox);
              static Color s_Color(255, 255, 255, 255);

              const Vec2& boxPos = textBox.GetPos();
              const Vec2& boxSize = textBox.GetSize();
              float posX = std::max(boxPos[0], minX);
              float maxSize = boxPos[0] + boxSize[0] - posX;
              m_TextRendererStatic.AddTextTrailingCharsPrioritized(
                  textBox.GetText().c_str(), posX, textBox.GetPosY() + 1.f,
                  GlCanvas::Z_VALUE_TEXT, s_Color,
                  textBox.GetElapsedTimeTextLength(), maxSize);
            }
          } else {
            Line line;
            line.m_Beg = Vec3(pos[0], pos[1], z);
            line.m_End = Vec3(pos[0], pos[1] + size[1], z);
            Color colors[2];
            Fill(colors, col);
            m_Batcher.AddLine(line, colors, PickingID::LINE, &textBox);
          }
        }

        ++TextBoxID;
      }
    }
  }

  if (!a_Picking) {
    UpdateEvents();
  }

  m_NeedsUpdatePrimitives = false;
  m_NeedsRedraw = true;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateEvents() {
  TickType rawMin = GetTickFromUs(m_MinTimeUs);
  TickType rawMax = GetTickFromUs(m_MaxTimeUs);

  ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());

  Color lineColor[2];
  Color white(255, 255, 255, 255);
  Fill(lineColor, white);

  for (auto& pair : GEventTracer.GetEventBuffer().GetCallstacks()) {
    ThreadID threadID = pair.first;
    std::map<long long, CallstackEvent>& callstacks = pair.second;

    // Sampling Events
    float ThreadOffset = (float)m_Layout.GetSamplingTrackOffset(threadID);
    if (ThreadOffset != -1.f) {
      for (auto& callstackPair : callstacks) {
        unsigned long long time = callstackPair.first;

        if (time > rawMin && time < rawMax) {
          float x = GetWorldFromTick(time);
          Line line;
          line.m_Beg = Vec3(x, ThreadOffset, GlCanvas::Z_VALUE_EVENT);
          line.m_End = Vec3(x, ThreadOffset - m_Layout.GetEventTrackHeight(),
                            GlCanvas::Z_VALUE_EVENT);
          m_Batcher.AddLine(line, lineColor, PickingID::EVENT);
        }
      }
    }
  }

  // Draw selected events
  Color selectedColor[2];
  Color col(0, 255, 0, 255);
  Fill(selectedColor, col);
  for (CallstackEvent& event : m_SelectedCallstackEvents) {
    float x = GetWorldFromTick(event.m_Time);
    float ThreadOffset = (float)m_Layout.GetSamplingTrackOffset(event.m_TID);

    Line line;
    line.m_Beg = Vec3(x, ThreadOffset, GlCanvas::Z_VALUE_EVENT);
    line.m_End = Vec3(x, ThreadOffset - m_Layout.GetEventTrackHeight(),
                      GlCanvas::Z_VALUE_TEXT);
    m_Batcher.AddLine(line, selectedColor, PickingID::EVENT);
  }
}

//-----------------------------------------------------------------------------
void TimeGraph::SelectEvents(float a_WorldStart, float a_WorldEnd,
                             ThreadID a_TID) {
  if (a_WorldStart > a_WorldEnd) {
    std::swap(a_WorldEnd, a_WorldStart);
  }

  TickType t0 = GetTickFromWorld(a_WorldStart);
  TickType t1 = GetTickFromWorld(a_WorldEnd);

  m_SelectedCallstackEvents =
      GEventTracer.GetEventBuffer().GetCallstackEvents(t0, t1, a_TID);

  // Generate report
  std::shared_ptr<SamplingProfiler> samplingProfiler =
      std::make_shared<SamplingProfiler>(Capture::GTargetProcess);
  samplingProfiler->SetIsLinuxPerf(
      Capture::IsRemote());  // TODO: could be windows to windows remote
                             // capture...
  samplingProfiler->SetState(SamplingProfiler::Sampling);

  samplingProfiler->SetGenerateSummary(a_TID == 0);

  for (CallstackEvent& event : m_SelectedCallstackEvents) {
    const std::shared_ptr<CallStack> callstack =
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
}

//-----------------------------------------------------------------------------
void TimeGraph::Draw(bool a_Picking) {
  // TODO: keep() used to maintain a cap on the number of timers we store in
  // memory.
  //       Now that each thread track has multiple blockchains as opposed to
  //       only having a single global one, this a bit trickier.
  if (/*m_TextBoxes.keep( GParams.m_MaxNumTimers ) ||*/ (
          !a_Picking && m_NeedsUpdatePrimitives) ||
      a_Picking) {
    UpdatePrimitives(a_Picking);
  }

  DrawThreadTracks(a_Picking);
  DrawBuffered(a_Picking);
  DrawEvents(a_Picking);

  m_NeedsRedraw = false;
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawThreadTracks(bool a_Picking) {
  m_Layout.SetNumCores(GetNumCores());
  const std::vector<ThreadID>& sortedThreadIds = m_Layout.GetSortedThreadIds();
  for (uint32_t i = 0; i < sortedThreadIds.size(); ++i) {
    ThreadID threadId = sortedThreadIds[i];

    std::shared_ptr<ThreadTrack> track = GetThreadTrack(threadId);
    if (track->GetName().empty()) {
      std::string threadName =
          Capture::GTargetProcess->GetThreadNameFromTID(threadId);
      track->SetName(threadName);
    }

    track->Draw(m_Canvas, a_Picking);
  }
}

//-----------------------------------------------------------------------------
std::shared_ptr<ThreadTrack> TimeGraph::GetThreadTrack(ThreadID a_TID) {
  ScopeLock lock(m_Mutex);
  std::shared_ptr<ThreadTrack> track = m_ThreadTracks[a_TID];
  if (track == nullptr) {
    track = std::make_shared<ThreadTrack>(this, a_TID);
  }
  m_ThreadTracks[a_TID] = track;
  return track;
}

//-----------------------------------------------------------------------------
ThreadTrackMap TimeGraph::GetThreadTracksCopy() const {
  ScopeLock lock(m_Mutex);
  return m_ThreadTracks;
}

//-----------------------------------------------------------------------------
void TimeGraph::SetThreadFilter(const std::string& a_Filter) {
  std::cout << "Setting thread filter: " << a_Filter << std::endl;
  m_ThreadFilter = a_Filter;
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateThreadIds() {
  {
    ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
    m_EventCount.clear();

    for (auto& pair : GEventTracer.GetEventBuffer().GetCallstacks()) {
      ThreadID threadID = pair.first;
      std::map<long long, CallstackEvent>& callstacks = pair.second;

      m_EventCount[threadID] = (uint32_t)callstacks.size();
      GetThreadTrack(threadID);
    }
  }

  // Reorder threads once every second when capturing
  if (!Capture::IsCapturing() || m_LastThreadReorder.QueryMillis() > 1000.0) {
    std::vector<ThreadID> sortedThreadIds;

    // Show threads with instrumented functions first
    std::vector<std::pair<ThreadID, uint32_t>> sortedThreads =
        OrbitUtils::ReverseValueSort(m_ThreadCountMap);
    for (auto& pair : sortedThreads) {
      sortedThreadIds.push_back(pair.first);
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
        std::shared_ptr<ThreadTrack> track = GetThreadTrack(tid);

        for (auto& filter : filters) {
          if (track && Contains(track->GetName(), filter)) {
            filteredThreadIds.push_back(tid);
          }
        }
      }
      sortedThreadIds = filteredThreadIds;
    }

    m_Layout.SetSortedThreadIds(sortedThreadIds);
    m_LastThreadReorder.Reset();
  }

  ScopeLock lock(m_Mutex);
  m_Layout.CalculateOffsets(m_ThreadTracks);
}

//-----------------------------------------------------------------------------
Color TimeGraph::GetThreadColor(ThreadID a_TID) const {
  return ThreadTrack::GetColor(a_TID);
}

//----------------------------------------------------------------------------
void TimeGraph::OnLeft() {
  TextBox* selection = Capture::GSelectedTextBox;
  if (selection) {
    const Timer& timer = selection->GetTimer();
    const TextBox* left = GetThreadTrack(timer.m_TID)->GetLeft(selection);
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
    const TextBox* right = GetThreadTrack(timer.m_TID)->GetRight(selection);
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
    const TextBox* up = GetThreadTrack(timer.m_TID)->GetUp(selection);
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
    const TextBox* down = GetThreadTrack(timer.m_TID)->GetDown(selection);
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
void TimeGraph::DrawEvents(bool a_Picking) {
  // Draw track background
  float x0 = GetWorldFromTick(m_SessionMinCounter);
  float x1 = GetWorldFromTick(m_SessionMaxCounter);
  float sizeX = x1 - x0;

  for (uint32_t i = 0; i < m_Layout.GetSortedThreadIds().size(); ++i) {
    ThreadID threadId = m_Layout.GetSortedThreadIds()[i];
    float y0 = m_Layout.GetSamplingTrackOffset(threadId);

    if (y0 == -1.f) continue;

    EventTrack* eventTrack = m_EventTracks[threadId];

    if (eventTrack) {
      eventTrack->SetPos(x0, y0);
      eventTrack->SetSize(sizeX, m_Layout.GetEventTrackHeight());
      eventTrack->Draw(m_Canvas, a_Picking);
    }
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

  double span = m_MaxTimeUs - m_MinTimeUs;
  double startUs = m_MinTimeUs + m_MarginRatio * span;

  if (startUs > end || m_MaxTimeUs < start) {
    return false;
  }

  return true;
}
