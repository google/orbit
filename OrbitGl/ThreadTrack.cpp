#include "ThreadTrack.h"

#include <limits>

#include "Capture.h"
#include "EventTrack.h"
#include "GlCanvas.h"
#include "OrbitUnreal.h"
#include "Systrace.h"
#include "TimeGraph.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

// TODO: Remove this flag once we have a way to toggle the display return values
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");

//-----------------------------------------------------------------------------
ThreadTrack::ThreadTrack(TimeGraph* time_graph, uint32_t thread_id) {
  time_graph_ = time_graph;
  m_ID = thread_id;
  text_renderer_ = time_graph->GetTextRenderer();
  thread_id_ = thread_id;

  num_timers_ = 0;
  min_time_ = std::numeric_limits<TickType>::max();
  max_time_ = std::numeric_limits<TickType>::min();

  event_track_ = std::make_shared<EventTrack>(time_graph);
  event_track_->SetThreadId(thread_id);
}

//-----------------------------------------------------------------------------
void ThreadTrack::Draw(GlCanvas* canvas, bool picking) {
  float track_height = GetHeight();
  float track_width = canvas->GetWorldWidth();

  SetPos(canvas->GetWorldTopLeftX(), m_Pos[1]);
  SetSize(track_width, track_height);

  Track::Draw(canvas, picking);

  // Event track
  float event_track_height = time_graph_->GetLayout().GetEventTrackHeight();
  event_track_->SetPos(m_Pos[0], m_Pos[1]);
  event_track_->SetSize(track_width, event_track_height);
  event_track_->Draw(canvas, picking);
}

//-----------------------------------------------------------------------------
std::string GetExtraInfo(const Timer& timer) {
  std::string info;
  static bool show_return_value = absl::GetFlag(FLAGS_show_return_values);
  if (!Capture::IsCapturing() && timer.GetType() == Timer::UNREAL_OBJECT) {
    info = "[" + ws2s(GOrbitUnreal.GetObjectNames()[timer.m_UserData[0]]) + "]";
  } else if (show_return_value && (timer.GetType() == Timer::NONE)) {
    info = absl::StrFormat("[%lu]", timer.m_UserData[0]);
  }
  return info;
}

//-----------------------------------------------------------------------------
void ThreadTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) {
  event_track_->UpdatePrimitives(min_tick, max_tick);
  Batcher* batcher = &time_graph_->GetBatcher();
  TextRenderer* text_renderer = time_graph_->GetTextRenderer();
  GlCanvas* canvas = time_graph_->GetCanvas();

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const TextBox& scene_box = canvas->GetSceneBox();
  float min_x = scene_box.GetPosX();

  double time_window_us = time_graph_->GetTimeWindowUs();
  float world_start_x = canvas->GetWorldTopLeftX();
  float world_width = canvas->GetWorldWidth();
  double inv_time_window = 1.0 / time_window_us;

  std::vector<std::shared_ptr<TimerChain>> depth_chain = GetTimers();
  for (auto& text_boxes : depth_chain) {
    if (text_boxes == nullptr) continue;

    for (TextBox& text_box : *text_boxes) {
      const Timer& timer = text_box.GetTimer();

      if (!(min_tick > timer.m_End || max_tick < timer.m_Start)) {
        double start = time_graph_->GetUsFromTick(timer.m_Start);
        double end = time_graph_->GetUsFromTick(timer.m_End);
        double elapsed = end - start;

        double normalized_start = start * inv_time_window;
        double normalized_length = elapsed * inv_time_window;

        bool is_core = timer.IsType(Timer::CORE_ACTIVITY);

        float y_offset = 0;
        if (!is_core) {
          y_offset = m_Pos[1] - layout.GetEventTrackHeight() -
                     layout.GetSpaceBetweenTracksAndThread() -
                     layout.GetTextBoxHeight() * (timer.m_Depth + 1);
        } else {
          y_offset = layout.GetCoreOffset(timer.m_Processor);
        }

        float box_height =
            !is_core ? layout.GetTextBoxHeight() : layout.GetTextCoresHeight();

        float world_timer_start_x =
            float(world_start_x + normalized_start * world_width);
        float world_timer_width = float(normalized_length * world_width);

        Vec2 pos(world_timer_start_x, y_offset);
        Vec2 size(world_timer_width, box_height);

        text_box.SetPos(pos);
        text_box.SetSize(size);

        if (!is_core) {
          time_graph_->UpdateThreadDepth(timer.m_TID, timer.m_Depth + 1);
          UpdateDepth(timer.m_Depth + 1);
        } else {
          UpdateDepth(timer.m_Processor + 1);
        }

        bool is_context_switch = timer.IsType(Timer::THREAD_ACTIVITY);
        bool is_visible_width = normalized_length * canvas->getWidth() > 1;
        bool is_same_pid_as_target =
            is_core && Capture::GTargetProcess != nullptr
                ? timer.m_PID == Capture::GTargetProcess->GetID()
                : true;
        bool is_same_tid_as_selected =
            is_core && (timer.m_TID == Capture::GSelectedThreadId);
        bool is_inactive =
            (!is_context_switch && timer.m_FunctionAddress &&
             (!Capture::GVisibleFunctionsMap.empty() &&
              Capture::GVisibleFunctionsMap[timer.m_FunctionAddress] ==
                  nullptr)) ||
            (Capture::GSelectedThreadId != 0 && is_core &&
             !is_same_tid_as_selected);
        bool is_selected = &text_box == Capture::GSelectedTextBox;

        const unsigned char g = 100;
        Color grey(g, g, g, 255);
        static Color selection_color(0, 128, 255, 255);

        Color col = time_graph_->GetTimesliceColor(timer);

        if (is_selected) {
          col = selection_color;
        } else if (!is_same_tid_as_selected &&
                   (is_inactive || !is_same_pid_as_target)) {
          col = grey;
        }

        text_box.SetColor(col[0], col[1], col[2]);
        static int oddAlpha = 210;
        if (!(timer.m_Depth & 0x1)) {
          col[3] = oddAlpha;
        }

        float z = is_inactive ? GlCanvas::Z_VALUE_BOX_INACTIVE
                              : GlCanvas::Z_VALUE_BOX_ACTIVE;

        if (is_visible_width) {
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
          batcher->AddBox(box, colors, PickingID::BOX, &text_box);

          if (!is_context_switch && text_box.GetText().empty()) {
            double elapsed_millis = ((double)elapsed) * 0.001;
            std::string time = GetPrettyTime(elapsed_millis);
            Function* func =
                Capture::GSelectedFunctionsMap[timer.m_FunctionAddress];

            text_box.SetElapsedTimeTextLength(time.length());

            const char* name = nullptr;
            if (func) {
              std::string extra_info = GetExtraInfo(timer);
              name = func->PrettyName().c_str();
              std::string text = absl::StrFormat(
                  "%s %s %s", name, extra_info.c_str(), time.c_str());

              text_box.SetText(text);
            } else if (timer.m_Type == Timer::INTROSPECTION) {
              std::string text = absl::StrFormat("%s %s",
                                                 time_graph_->GetStringManager()
                                                     ->Get(timer.m_UserData[0])
                                                     .value_or(""),
                                                 time.c_str());
              text_box.SetText(text);
            } else if (timer.m_Type == Timer::GPU_ACTIVITY) {
              std::string text =
                  absl::StrFormat("%s; submitter: %d  %s",
                                  time_graph_->GetStringManager()
                                      ->Get(timer.m_UserData[0])
                                      .value_or(""),
                                  timer.m_SubmitTID, time.c_str());
              text_box.SetText(text);
            } else if (!SystraceManager::Get().IsEmpty()) {
              text_box.SetText(SystraceManager::Get().GetFunctionName(
                  timer.m_FunctionAddress));
            } else if (!Capture::IsCapturing()) {
              // GZoneNames is populated when capturing, prevent race
              // by accessing it only when not capturing.
              auto it = Capture::GZoneNames.find(timer.m_FunctionAddress);
              if (it != Capture::GZoneNames.end()) {
                name = it->second.c_str();
                std::string text = absl::StrFormat("%s %s", name, time.c_str());
                text_box.SetText(text);
              }
            }
          }

          if (!is_core) {
            static Color s_Color(255, 255, 255, 255);

            const Vec2& box_pos = text_box.GetPos();
            const Vec2& box_size = text_box.GetSize();
            float pos_x = std::max(box_pos[0], min_x);
            float max_size = box_pos[0] + box_size[0] - pos_x;
            text_renderer->AddTextTrailingCharsPrioritized(
                text_box.GetText().c_str(), pos_x, text_box.GetPosY() + 1.f,
                GlCanvas::Z_VALUE_TEXT, s_Color,
                text_box.GetElapsedTimeTextLength(), max_size);
          }
        } else {
          Line line;
          line.m_Beg = Vec3(pos[0], pos[1], z);
          line.m_End = Vec3(pos[0], pos[1] + size[1], z);
          Color colors[2];
          Fill(colors, col);
          batcher->AddLine(line, colors, PickingID::LINE, &text_box);
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void ThreadTrack::OnDrag(int x, int y) { Track::OnDrag(x, y); }

//-----------------------------------------------------------------------------
void ThreadTrack::OnTimer(const Timer& timer) {
  if (timer.m_Type != Timer::CORE_ACTIVITY) {
    UpdateDepth(timer.m_Depth + 1);
  }

  TextBox text_box(Vec2(0, 0), Vec2(0, 0), "", Color(255, 0, 0, 255));
  text_box.SetTimer(timer);

  std::shared_ptr<TimerChain> timer_chain = timers_[timer.m_Depth];
  if (timer_chain == nullptr) {
    timer_chain = std::make_shared<TimerChain>();
    timers_[timer.m_Depth] = timer_chain;
  }
  timer_chain->push_back(text_box);
  ++num_timers_;
  if (timer.m_Start < min_time_) min_time_ = timer.m_Start;
  if (timer.m_End > max_time_) max_time_ = timer.m_End;
}

//-----------------------------------------------------------------------------
float ThreadTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  return layout.GetTextBoxHeight() * GetDepth() +
         layout.GetSpaceBetweenTracksAndThread() +
         layout.GetEventTrackHeight() + layout.GetTrackBottomMargin();
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> ThreadTrack::GetTimers() {
  std::vector<std::shared_ptr<TimerChain>> timers;
  ScopeLock lock(mutex_);
  for (auto& pair : timers_) {
    timers.push_back(pair.second);
  }
  return timers;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetFirstAfterTime(TickType time,
                                              uint32_t depth) const {
  std::shared_ptr<TimerChain> text_boxes = GetTimers(depth);
  if (text_boxes == nullptr) return nullptr;

  // TODO: do better than linear search...
  for (TextBox& text_box : *text_boxes) {
    if (text_box.GetTimer().m_Start > time) {
      return &text_box;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetFirstBeforeTime(TickType time,
                                               uint32_t depth) const {
  std::shared_ptr<TimerChain> text_boxes = GetTimers(depth);
  if (text_boxes == nullptr) return nullptr;

  TextBox* text_box = nullptr;

  // TODO: do better than linear search...
  for (TextBox& box : *text_boxes) {
    if (box.GetTimer().m_Start > time) {
      return text_box;
    }

    text_box = &box;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<TimerChain> ThreadTrack::GetTimers(uint32_t depth) const {
  ScopeLock lock(mutex_);
  auto it = timers_.find(depth);
  if (it != timers_.end()) return it->second;
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetLeft(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  if (timer.m_TID == thread_id_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer.m_Depth);
    if (timers) return timers->GetElementBefore(text_box);
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetRight(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  if (timer.m_TID == thread_id_) {
    std::shared_ptr<TimerChain> timers = GetTimers(timer.m_Depth);
    if (timers) return timers->GetElementAfter(text_box);
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetUp(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  return GetFirstBeforeTime(timer.m_Start, timer.m_Depth - 1);
}

//-----------------------------------------------------------------------------
const TextBox* ThreadTrack::GetDown(TextBox* text_box) const {
  const Timer& timer = text_box->GetTimer();
  return GetFirstAfterTime(timer.m_Start, timer.m_Depth + 1);
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<TimerChain>> ThreadTrack::GetAllChains() {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& pair : timers_) {
    chains.push_back(pair.second);
  }
  return chains;
}

//-----------------------------------------------------------------------------
void ThreadTrack::SetEventTrackColor(Color color) {
  ScopeLock lock(mutex_);
  event_track_->SetColor(color);
}
