#include "SchedulerTrack.h"

#include "Capture.h"
#include "EventTrack.h"
#include "GlCanvas.h"
#include "TimeGraph.h"

const Color kInactiveColor(100, 100, 100, 255);
const Color kSelectionColor(0, 128, 255, 255);

SchedulerTrack::SchedulerTrack(TimeGraph* time_graph)
    : ThreadTrack(time_graph, /*thread_id*/ 0) {}

void SchedulerTrack::Draw(GlCanvas* /*canvas*/, bool /*picking*/) {}

float SchedulerTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  return depth_ *
             (layout.GetTextCoresHeight() + layout.GetSpaceBetweenCores()) +
         layout.GetTrackBottomMargin();
}

inline Color GetTimerColor(const Timer& timer, TimeGraph* time_graph,
                           bool is_selected, bool same_tid, bool same_pid,
                           bool inactive) {
  if (is_selected) {
    return kSelectionColor;
  } else if (!same_tid && (inactive || !same_pid)) {
    return kInactiveColor;
  }
  return time_graph->GetThreadColor(timer.m_TID);
}

float SchedulerTrack::GetYFromDepth(float track_y, uint32_t depth,
                                    bool /*collapsed*/) {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  return track_y - layout.GetSpaceBetweenTracksAndThread() -
         (layout.GetTextCoresHeight() + layout.GetSpaceBetweenCores()) *
             (depth + 1);
}

void SchedulerTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) {
  Batcher* batcher = &time_graph_->GetBatcher();
  GlCanvas* canvas = time_graph_->GetCanvas();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  const auto& target_process = Capture::GTargetProcess;
  uint32_t target_pid = target_process ? target_process->GetID() : 0;
  uint32_t selected_thread_id = Capture::GSelectedThreadId;

  float world_start_x = canvas->GetWorldTopLeftX();
  float world_width = canvas->GetWorldWidth();
  double inv_time_window = 1.0 / time_graph_->GetTimeWindowUs();

  std::vector<std::shared_ptr<TimerChain>> chains_by_depth = GetTimers();
  for (std::shared_ptr<TimerChain>& timer_chain : chains_by_depth) {
    if (timer_chain == nullptr) continue;
    for (TextBox& text_box : *timer_chain) {
      const Timer& timer = text_box.GetTimer();
      if (min_tick > timer.m_End || max_tick < timer.m_Start) continue;

      UpdateDepth(timer.m_Depth + 1);
      double start_us = time_graph_->GetUsFromTick(timer.m_Start);
      double end_us = time_graph_->GetUsFromTick(timer.m_End);
      double elapsed_us = end_us - start_us;
      double normalized_start = start_us * inv_time_window;
      double normalized_length = elapsed_us * inv_time_window;
      float world_timer_width =
          static_cast<float>(normalized_length * world_width);
      float world_timer_x =
          static_cast<float>(world_start_x + normalized_start * world_width);
      float world_timer_y =
          GetYFromDepth(m_Pos[1], timer.m_Depth, /*collapsed*/ false);

      bool is_visible_width = normalized_length * canvas->getWidth() > 1;
      bool is_same_pid_as_target = target_pid == 0 || target_pid == timer.m_PID;
      bool is_same_tid_as_selected = timer.m_TID == selected_thread_id;
      bool is_inactive = selected_thread_id != 0 && !is_same_tid_as_selected;
      bool is_selected = &text_box == Capture::GSelectedTextBox;

      Vec2 pos(world_timer_x, world_timer_y);
      Vec2 size(world_timer_width, layout.GetTextCoresHeight());
      float z = GlCanvas::Z_VALUE_BOX_ACTIVE;
      Color color = GetTimerColor(timer, time_graph_, is_selected,
                                  is_same_tid_as_selected,
                                  is_same_pid_as_target, is_inactive);

      if (is_visible_width) {
        batcher->AddShadedBox(pos, size, z, color, PickingID::BOX, &text_box);
      } else {
        auto type = PickingID::LINE;
        batcher->AddVerticalLine(pos, size[1], z, color, type, &text_box);
      }
    }
  }
}
