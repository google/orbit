// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_H_
#define ORBIT_GL_TIME_GRAPH_H_

#include <unordered_map>
#include <utility>

#include "AsyncTrack.h"
#include "Batcher.h"
#include "BlockChain.h"
#include "FrameTrack.h"
#include "Geometry.h"
#include "GpuTrack.h"
#include "GraphTrack.h"
#include "ManualInstrumentationManager.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Tracing.h"
#include "SchedulerTrack.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "ThreadTrack.h"
#include "TimeGraphLayout.h"
#include "Timer.h"
#include "TimerChain.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class TimeGraph {
 public:
  explicit TimeGraph(uint32_t font_size);
  ~TimeGraph();

  void Draw(GlCanvas* canvas, PickingMode picking_mode = PickingMode::kNone);
  void DrawTracks(GlCanvas* canvas, PickingMode picking_mode = PickingMode::kNone);
  void DrawOverlay(GlCanvas* canvas, PickingMode picking_mode);
  void DrawText(GlCanvas* canvas, float layer);

  void NeedsUpdate();
  void UpdatePrimitives(PickingMode picking_mode);
  void SortTracks();
  void UpdateFilteredTrackList();
  void UpdateMovingTrackSorting();
  void UpdateTracks(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode);
  void SelectEvents(float world_start, float world_end, int32_t thread_id);
  const std::vector<orbit_client_protos::CallstackEvent>& GetSelectedCallstackEvents(int32_t tid);

  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info,
                    const orbit_client_protos::FunctionInfo* function);
  void UpdateMaxTimeStamp(uint64_t time);

  [[nodiscard]] float GetThreadTotalHeight() const;
  [[nodiscard]] float GetTextBoxHeight() const { return layout_.GetTextBoxHeight(); }
  [[nodiscard]] float GetWorldFromTick(uint64_t time) const;
  [[nodiscard]] float GetWorldFromUs(double micros) const;
  [[nodiscard]] uint64_t GetTickFromWorld(float world_x) const;
  [[nodiscard]] uint64_t GetTickFromUs(double micros) const;
  [[nodiscard]] double GetUsFromTick(uint64_t time) const;
  [[nodiscard]] double GetTimeWindowUs() const { return time_window_us_; }
  void GetWorldMinMax(float& min, float& max) const;
  [[nodiscard]] bool UpdateCaptureMinMaxTimestamps();

  void Clear();
  void ZoomAll();
  void Zoom(const orbit_client_protos::TimerInfo& timer_info);
  void Zoom(uint64_t min, uint64_t max);
  void ZoomTime(float zoom_value, double mouse_ratio);
  void VerticalZoom(float zoom_value, float mouse_ratio);
  void SetMinMax(double min_time_us, double max_time_us);
  void PanTime(int initial_x, int current_x, int width, double initial_time);
  enum class VisibilityType {
    kPartlyVisible,
    kFullyVisible,
  };
  void HorizontallyMoveIntoView(VisibilityType vis_type, uint64_t min, uint64_t max,
                                double distance = 0.3);
  void HorizontallyMoveIntoView(VisibilityType vis_type,
                                const orbit_client_protos::TimerInfo& timer_info,
                                double distance = 0.3);
  void VerticallyMoveIntoView(const orbit_client_protos::TimerInfo& timer_info);
  void VerticallyMoveIntoView(Track& track);

  [[nodiscard]] double GetTime(double ratio) const;
  void Select(const TextBox* text_box);
  enum class JumpScope { kGlobal, kSameDepth, kSameThread, kSameFunction, kSameThreadSameFunction };
  enum class JumpDirection { kPrevious, kNext, kTop, kDown };
  void JumpToNeighborBox(const TextBox* from, JumpDirection jump_direction, JumpScope jump_scope);
  [[nodiscard]] const TextBox* FindPreviousFunctionCall(
      uint64_t function_address, uint64_t current_time,
      std::optional<int32_t> thread_id = std::nullopt) const;
  [[nodiscard]] const TextBox* FindNextFunctionCall(
      uint64_t function_address, uint64_t current_time,
      std::optional<int32_t> thread_id = std::nullopt) const;
  void SelectAndZoom(const TextBox* text_box);
  [[nodiscard]] double GetCaptureTimeSpanUs();
  [[nodiscard]] double GetCurrentTimeSpanUs() const;
  void NeedsRedraw() { needs_redraw_ = true; }
  [[nodiscard]] bool IsRedrawNeeded() const { return needs_redraw_; }
  void SetThreadFilter(const std::string& filter);

  [[nodiscard]] bool IsFullyVisible(uint64_t min, uint64_t max) const;
  [[nodiscard]] bool IsPartlyVisible(uint64_t min, uint64_t max) const;
  [[nodiscard]] bool IsVisible(VisibilityType vis_type, uint64_t min, uint64_t max) const;

  [[nodiscard]] int GetNumDrawnTextBoxes() { return num_drawn_text_boxes_; }
  void SetTextRenderer(TextRenderer* text_renderer) { text_renderer_ = text_renderer; }
  [[nodiscard]] TextRenderer* GetTextRenderer() { return &text_renderer_static_; }
  void SetStringManager(std::shared_ptr<StringManager> str_manager);
  [[nodiscard]] StringManager* GetStringManager() { return string_manager_.get(); }
  void SetCanvas(GlCanvas* canvas);
  [[nodiscard]] GlCanvas* GetCanvas() { return canvas_; }
  void SetFontSize(uint32_t font_size);
  [[nodiscard]] uint32_t GetFontSize() const { return text_renderer_static_.GetFontSize(); }
  [[nodiscard]] uint32_t CalculateZoomedFontSize() const {
    return lround((font_size_)*layout_.GetScale());
  }
  [[nodiscard]] Batcher& GetBatcher() { return batcher_; }
  [[nodiscard]] uint32_t GetNumTimers() const;
  [[nodiscard]] uint32_t GetNumCores() const { return num_cores_; }
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllTimerChains() const;
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllThreadTrackTimerChains() const;
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllSerializableTimerChains() const;

  void UpdateHorizontalScroll(float ratio);
  [[nodiscard]] double GetMinTimeUs() const { return min_time_us_; }
  [[nodiscard]] double GetMaxTimeUs() const { return max_time_us_; }
  [[nodiscard]] const TimeGraphLayout& GetLayout() const { return layout_; }
  [[nodiscard]] TimeGraphLayout& GetLayout() { return layout_; }
  [[nodiscard]] float GetRightMargin() const { return right_margin_; }
  void UpdateRightMargin(float margin);

  [[nodiscard]] const TextBox* FindPrevious(const TextBox* from);
  [[nodiscard]] const TextBox* FindNext(const TextBox* from);
  [[nodiscard]] const TextBox* FindTop(const TextBox* from);
  [[nodiscard]] const TextBox* FindDown(const TextBox* from);

  [[nodiscard]] static Color GetColor(uint32_t id) {
    constexpr unsigned char kAlpha = 255;
    static std::vector<Color> colors{
        Color(231, 68, 53, kAlpha),    // red
        Color(43, 145, 175, kAlpha),   // blue
        Color(185, 117, 181, kAlpha),  // purple
        Color(87, 166, 74, kAlpha),    // green
        Color(215, 171, 105, kAlpha),  // beige
        Color(248, 101, 22, kAlpha)    // orange
    };
    return colors[id % colors.size()];
  }
  [[nodiscard]] static Color GetColor(uint64_t id) { return GetColor(static_cast<uint32_t>(id)); }
  [[nodiscard]] static Color GetColor(const std::string& str) { return GetColor(StringHash(str)); }
  [[nodiscard]] static Color GetThreadColor(int32_t tid) {
    return GetColor(static_cast<uint32_t>(tid));
  }

  void SetIteratorOverlayData(
      const absl::flat_hash_map<uint64_t, const TextBox*>& iterator_text_boxes,
      const absl::flat_hash_map<uint64_t, const orbit_client_protos::FunctionInfo*>&
          iterator_functions) {
    iterator_text_boxes_ = iterator_text_boxes;
    iterator_functions_ = iterator_functions;
    NeedsRedraw();
  }

  void DrawIteratorBox(GlCanvas* canvas, Vec2 pos, Vec2 size, const Color& color,
                       const std::string& label, const std::string& time, float text_box_y);

  [[nodiscard]] uint64_t GetCaptureMin() const { return capture_min_timestamp_; }
  [[nodiscard]] uint64_t GetCaptureMax() const { return capture_max_timestamp_; }
  [[nodiscard]] uint64_t GetCurrentMouseTimeNs() const { return current_mouse_time_ns_; }

  void RemoveFrameTrack(const orbit_client_protos::FunctionInfo& function);

 protected:
  std::shared_ptr<SchedulerTrack> GetOrCreateSchedulerTrack();
  std::shared_ptr<ThreadTrack> GetOrCreateThreadTrack(int32_t tid);
  std::shared_ptr<GpuTrack> GetOrCreateGpuTrack(uint64_t timeline_hash);
  GraphTrack* GetOrCreateGraphTrack(const std::string& name);
  AsyncTrack* GetOrCreateAsyncTrack(const std::string& name);
  std::shared_ptr<FrameTrack> GetOrCreateFrameTrack(
      const orbit_client_protos::FunctionInfo& function);

  void AddTrack(std::shared_ptr<Track> track);
  int FindMovingTrackIndex();

  [[nodiscard]] std::vector<int32_t> GetSortedThreadIds();

  void ProcessOrbitFunctionTimer(orbit_client_protos::FunctionInfo::OrbitType type,
                                 const orbit_client_protos::TimerInfo& timer_info);
  void ProcessIntrospectionTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessValueTrackingTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessAsyncTimer(const std::string& track_name,
                         const orbit_client_protos::TimerInfo& timer_info);
  void SetNumCores(uint32_t num_cores) { num_cores_ = num_cores; }

 private:
  uint32_t font_size_;
  TextRenderer text_renderer_static_;
  TextRenderer* text_renderer_ = nullptr;
  GlCanvas* canvas_ = nullptr;
  int num_drawn_text_boxes_ = 0;
  bool sorting_invalidated_ = true;

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
  float right_margin_ = 0;

  TimeGraphLayout layout_;

  std::map<int32_t, uint32_t> thread_count_map_;
  uint32_t num_cores_;
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
  std::map<std::string, std::shared_ptr<AsyncTrack>> async_tracks_;
  std::map<std::string, std::shared_ptr<GraphTrack>> graph_tracks_;
  // Mapping from timeline hash to GPU tracks.
  std::unordered_map<uint64_t, std::shared_ptr<GpuTrack>> gpu_tracks_;
  // Mapping from function address to frame tracks.
  std::unordered_map<uint64_t, std::shared_ptr<FrameTrack>> frame_tracks_;

  std::vector<std::shared_ptr<Track>> sorted_tracks_;
  std::vector<std::shared_ptr<Track>> sorted_filtered_tracks_;
  std::string thread_filter_;

  std::shared_ptr<SchedulerTrack> scheduler_track_;
  std::shared_ptr<ThreadTrack> tracepoints_system_wide_track_;

  absl::flat_hash_map<int32_t, std::vector<orbit_client_protos::CallstackEvent>>
      selected_callstack_events_per_thread_;

  std::shared_ptr<StringManager> string_manager_;
  ManualInstrumentationManager* manual_instrumentation_manager_;
  std::unique_ptr<ManualInstrumentationManager::AsyncTimerInfoListener> async_timer_info_listener_;
};

extern TimeGraph* GCurrentTimeGraph;

#endif  // ORBIT_GL_TIME_GRAPH_H_
