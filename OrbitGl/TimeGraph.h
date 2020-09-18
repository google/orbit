// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_H_
#define ORBIT_GL_TIME_GRAPH_H_

#include <unordered_map>
#include <utility>

#include "Batcher.h"
#include "BlockChain.h"
#include "Geometry.h"
#include "GpuTrack.h"
#include "GraphTrack.h"
#include "OrbitBase/Profiling.h"
#include "SchedulerTrack.h"
#include "ScopeTimer.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class TimeGraph {
 public:
  TimeGraph();

  void Draw(GlCanvas* canvas, PickingMode picking_mode = PickingMode::kNone);
  void DrawTracks(GlCanvas* canvas, PickingMode picking_mode = PickingMode::kNone);
  void DrawOverlay(GlCanvas* canvas, PickingMode picking_mode);
  void DrawText(GlCanvas* canvas);

  void NeedsUpdate();
  void UpdatePrimitives(PickingMode picking_mode);
  void SortTracks();
  std::vector<orbit_client_protos::CallstackEvent> SelectEvents(float world_start, float world_end,
                                                                int32_t thread_id);
  const std::vector<orbit_client_protos::CallstackEvent>& GetSelectedCallstackEvents(int32_t tid);

  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info,
                    const orbit_client_protos::FunctionInfo* function);
  void ProcessValueTrackingTimer(const orbit_client_protos::TimerInfo& timer_info);
  void UpdateMaxTimeStamp(uint64_t a_Time);

  float GetThreadTotalHeight();
  float GetTextBoxHeight() const { return layout_.GetTextBoxHeight(); }
  int GetMarginInPixels() const { return margin_; }
  float GetWorldFromTick(uint64_t a_Time) const;
  float GetWorldFromUs(double a_Micros) const;
  uint64_t GetTickFromWorld(float a_WorldX) const;
  uint64_t GetTickFromUs(double a_MicroSeconds) const;
  double GetUsFromTick(uint64_t time) const;
  double GetTimeWindowUs() const { return time_window_us_; }
  void GetWorldMinMax(float& a_Min, float& a_Max) const;
  bool UpdateCaptureMinMaxTimestamps();

  void Clear();
  void ZoomAll();
  void Zoom(const TextBox* a_TextBox);
  void Zoom(uint64_t min, uint64_t max);
  void ZoomTime(float a_ZoomValue, double a_MouseRatio);
  void VerticalZoom(float a_ZoomValue, float a_MouseRatio);
  void SetMinMax(double a_MinTimeUs, double a_MaxTimeUs);
  void PanTime(int a_InitialX, int a_CurrentX, int a_Width, double a_InitialTime);
  enum class VisibilityType {
    kPartlyVisible,
    kFullyVisible,
  };
  void HorizontallyMoveIntoView(VisibilityType vis_type, uint64_t min, uint64_t max,
                                double distance = 0.3);
  void HorizontallyMoveIntoView(VisibilityType vis_type, const TextBox* text_box,
                                double distance = 0.3);
  void VerticallyMoveIntoView(const TextBox* text_box);
  double GetTime(double a_Ratio) const;
  double GetTimeIntervalMicro(double a_Ratio);
  void Select(const TextBox* text_box);
  enum class JumpScope { kGlobal, kSameDepth, kSameThread, kSameFunction, kSameThreadSameFunction };
  enum class JumpDirection { kPrevious, kNext, kTop, kDown };
  void JumpToNeighborBox(const TextBox* from, JumpDirection jump_direction, JumpScope jump_scope);
  const TextBox* FindPreviousFunctionCall(uint64_t function_address, uint64_t current_time,
                                          std::optional<int32_t> thread_ID = std::nullopt) const;
  const TextBox* FindNextFunctionCall(uint64_t function_address, uint64_t current_time,
                                      std::optional<int32_t> thread_ID = std::nullopt) const;
  void SelectAndZoom(const TextBox* a_TextBox);
  double GetCaptureTimeSpanUs();
  double GetCurrentTimeSpanUs();
  void NeedsRedraw() { needs_redraw_ = true; }
  bool IsRedrawNeeded() const { return needs_redraw_; }
  void ToggleDrawText() { draw_text_ = !draw_text_; }
  void SetThreadFilter(const std::string& a_Filter);

  bool IsFullyVisible(uint64_t min, uint64_t max) const;
  bool IsPartlyVisible(uint64_t min, uint64_t max) const;
  bool IsVisible(VisibilityType vis_type, uint64_t min, uint64_t max) const;

  int GetNumDrawnTextBoxes() { return num_drawn_text_boxes_; }
  void SetTextRenderer(TextRenderer* a_TextRenderer) { text_renderer_ = a_TextRenderer; }
  TextRenderer* GetTextRenderer() { return &text_renderer_static_; }
  void SetStringManager(std::shared_ptr<StringManager> str_manager);
  StringManager* GetStringManager() { return string_manager_.get(); }
  void SetCanvas(GlCanvas* a_Canvas);
  GlCanvas* GetCanvas() { return canvas_; }
  void SetFontSize(int a_FontSize);
  int GetFontSize() { return GetTextRenderer()->GetFontSize(); }
  Batcher& GetBatcher() { return batcher_; }
  uint32_t GetNumTimers() const;
  uint32_t GetNumCores() const;
  std::vector<std::shared_ptr<TimerChain>> GetAllTimerChains() const;
  std::vector<std::shared_ptr<TimerChain>> GetAllThreadTrackTimerChains() const;

  void OnDrag(float a_Ratio);
  double GetMinTimeUs() const { return min_time_us_; }
  double GetMaxTimeUs() const { return max_time_us_; }
  const TimeGraphLayout& GetLayout() const { return layout_; }
  TimeGraphLayout& GetLayout() { return layout_; }
  float GetRightMargin() const { return right_margin_; }
  void SetRightMargin(float margin) { right_margin_ = margin; }

  const TextBox* FindPrevious(const TextBox* from);
  const TextBox* FindNext(const TextBox* from);
  const TextBox* FindTop(const TextBox* from);
  const TextBox* FindDown(const TextBox* from);

  Color GetThreadColor(int32_t tid) const;

  void SetIteratorOverlayData(
      const absl::flat_hash_map<uint64_t, const TextBox*>& iterator_text_boxes,
      const absl::flat_hash_map<uint64_t, const orbit_client_protos::FunctionInfo*>&
          iterator_functions) {
    iterator_text_boxes_ = iterator_text_boxes;
    iterator_functions_ = iterator_functions;
    NeedsRedraw();
  }

  uint64_t GetCaptureMin() const { return capture_min_timestamp_; }
  uint64_t GetCaptureMax() const { return capture_max_timestamp_; }
  uint64_t GetCurrentMouseTimeNs() const { return current_mouse_time_ns_; }

 protected:
  std::shared_ptr<SchedulerTrack> GetOrCreateSchedulerTrack();
  std::shared_ptr<ThreadTrack> GetOrCreateThreadTrack(int32_t tid);
  std::shared_ptr<GpuTrack> GetOrCreateGpuTrack(uint64_t timeline_hash);
  GraphTrack* GetOrCreateGraphTrack(const std::string& name);

  void ProcessManualIntrumentationTimer(const orbit_client_protos::TimerInfo& timer_info);

 private:
  TextRenderer text_renderer_static_;
  TextRenderer* text_renderer_ = nullptr;
  GlCanvas* canvas_ = nullptr;
  int num_drawn_text_boxes_ = 0;

  // First member is id.
  absl::flat_hash_map<uint64_t, const TextBox*> iterator_text_boxes_;
  absl::flat_hash_map<uint64_t, const orbit_client_protos::FunctionInfo*> iterator_functions_;

  double ref_time_us_ = 0;
  double min_time_us_ = 0;
  double max_time_us_ = 0;
  uint64_t capture_min_timestamp_ = 0;
  uint64_t capture_max_timestamp_ = 0;
  uint64_t current_mouse_time_ns_ = 0;
  std::map<int32_t, uint32_t> event_count_;
  double time_window_us_ = 0;
  float world_start_x_ = 0;
  float world_width_ = 0;
  float min_y_ = 0;
  int margin_ = 0;
  float right_margin_ = 0;

  double zoom_value_ = 0;
  double mouse_ratio_ = 0;

  TimeGraphLayout layout_;

  std::map<int32_t, uint32_t> thread_count_map_;

  // Be careful when directly changing these members without using the
  // methods NeedsRedraw() or NeedsUpdate():
  // needs_update_primitives_ should always imply needs_redraw_, that is
  // needs_update_primitives_ => needs_redraw_ is an invariant of this
  // class. When updating the primitives, which computes the primitives
  // to be drawn and their coordinates, we always have to redraw the
  // timeline.
  bool needs_update_primitives_ = false;
  bool needs_redraw_ = false;

  bool draw_text_ = true;

  Batcher batcher_;
  Timer last_thread_reorder_;

  mutable Mutex mutex_;
  std::vector<std::shared_ptr<Track>> tracks_;
  std::unordered_map<int32_t, std::shared_ptr<ThreadTrack>> thread_tracks_;
  std::map<std::string, std::shared_ptr<GraphTrack>> graph_tracks_;
  // Mapping from timeline hash to GPU tracks.
  std::unordered_map<uint64_t, std::shared_ptr<GpuTrack>> gpu_tracks_;
  std::vector<std::shared_ptr<Track>> sorted_tracks_;
  std::string thread_filter_;

  std::set<uint32_t> cores_seen_;
  std::shared_ptr<SchedulerTrack> scheduler_track_;
  std::shared_ptr<ThreadTrack> process_track_;

  absl::flat_hash_map<int32_t, std::vector<orbit_client_protos::CallstackEvent>>
      selected_callstack_events_per_thread_;

  std::shared_ptr<StringManager> string_manager_;
  StringManager manual_instrumentation_string_manager_;
};

extern TimeGraph* GCurrentTimeGraph;

#endif  // ORBIT_GL_TIME_GRAPH_H_
