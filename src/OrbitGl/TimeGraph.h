// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_H_
#define ORBIT_GL_TIME_GRAPH_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "AccessibleInterfaceProvider.h"
#include "Batcher.h"
#include "CaptureViewElement.h"
#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "CoreMath.h"
#include "ManualInstrumentationManager.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "PickingManager.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "TimelineInfoInterface.h"
#include "TimelineUi.h"
#include "TrackContainer.h"
#include "Viewport.h"

class OrbitApp;

class TimeGraph final : public orbit_gl::CaptureViewElement,
                        public orbit_gl::TimelineInfoInterface {
 public:
  explicit TimeGraph(AccessibleInterfaceProvider* parent, OrbitApp* app,
                     orbit_gl::Viewport* viewport, orbit_client_data::CaptureData* capture_data,
                     PickingManager* picking_manager);

  [[nodiscard]] float GetHeight() const override;

  void DrawAllElements(Batcher& batcher, TextRenderer& text_renderer, PickingMode& picking_mode,
                       uint64_t current_mouse_time_ns);
  void DrawText(float layer);

  void RequestUpdate() override;

  // TODO(b/214282122): Move Process Timers function outside the UI.
  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info,
                    const orbit_grpc_protos::InstrumentedFunction* function);
  void ProcessApiStringEvent(const orbit_client_protos::ApiStringEvent& string_event);
  void ProcessApiTrackValueEvent(const orbit_client_protos::ApiTrackValue& track_event);

  [[nodiscard]] const orbit_client_data::CaptureData* GetCaptureData() const {
    return capture_data_;
  }
  [[nodiscard]] orbit_gl::TrackContainer* GetTrackContainer() const {
    return track_container_.get();
  }
  [[nodiscard]] orbit_gl::TimelineUi* GetTimelineUi() const { return timeline_ui_.get(); }
  [[nodiscard]] orbit_gl::TrackManager* GetTrackManager() const {
    return track_container_->GetTrackManager();
  }

  [[nodiscard]] float GetWorldFromTick(uint64_t time) const override;
  [[nodiscard]] float GetWorldFromUs(double micros) const override;
  [[nodiscard]] uint64_t GetTickFromWorld(float world_x) const override;
  [[nodiscard]] uint64_t GetTickFromUs(double micros) const override;
  [[nodiscard]] double GetUsFromTick(uint64_t time) const override;
  [[nodiscard]] uint64_t GetNsSinceStart(uint64_t time) const override;
  [[nodiscard]] double GetTimeWindowUs() const override { return time_window_us_; }
  [[nodiscard]] double GetMinTimeUs() const override { return min_time_us_; }
  [[nodiscard]] double GetMaxTimeUs() const override { return max_time_us_; }

  void UpdateCaptureMinMaxTimestamps();

  void ZoomAll();
  void Zoom(const orbit_client_protos::TimerInfo& timer_info);
  void Zoom(uint64_t min, uint64_t max);
  void ZoomTime(float zoom_value, double mouse_ratio);
  void VerticalZoom(float zoom_value, float mouse_world_y_pos);
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

  [[nodiscard]] double GetTime(double ratio) const;
  void SelectAndMakeVisible(const orbit_client_protos::TimerInfo* timer_info);

  enum class JumpScope { kSameDepth, kSameThread, kSameFunction, kSameThreadSameFunction };
  enum class JumpDirection { kPrevious, kNext, kTop, kDown };
  void JumpToNeighborTimer(const orbit_client_protos::TimerInfo* from, JumpDirection jump_direction,
                           JumpScope jump_scope);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindPreviousFunctionCall(
      uint64_t function_address, uint64_t current_time,
      std::optional<uint32_t> thread_id = std::nullopt) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindNextFunctionCall(
      uint64_t function_address, uint64_t current_time,
      std::optional<uint32_t> thread_id = std::nullopt) const;
  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetAllTimersForHookedFunction(
      uint64_t function_address) const;
  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetAllThreadTrackTimerChains()
      const;
  [[nodiscard]] std::pair<const orbit_client_protos::TimerInfo*,
                          const orbit_client_protos::TimerInfo*>
  GetMinMaxTimerInfoForFunction(uint64_t function_id) const;

  void SelectAndZoom(const orbit_client_protos::TimerInfo* timer_info);
  [[nodiscard]] double GetCaptureTimeSpanUs() const;
  [[nodiscard]] double GetCurrentTimeSpanUs() const;
  [[nodiscard]] bool IsRedrawNeeded() const { return update_primitives_requested_; }

  [[nodiscard]] bool IsFullyVisible(uint64_t min, uint64_t max) const;
  [[nodiscard]] bool IsPartlyVisible(uint64_t min, uint64_t max) const;
  [[nodiscard]] bool IsVisible(VisibilityType vis_type, uint64_t min, uint64_t max) const;

  [[nodiscard]] TextRenderer* GetTextRenderer() { return &text_renderer_static_; }
  [[nodiscard]] Batcher& GetBatcher() { return batcher_; }

  void UpdateHorizontalScroll(float ratio);
  [[nodiscard]] const TimeGraphLayout& GetLayout() const { return layout_; }
  [[nodiscard]] TimeGraphLayout& GetLayout() { return layout_; }

  // TODO(http://b/194777907): Move GetColor outside TimeGraph
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
  [[nodiscard]] static Color GetColor(const std::string& str) {
    return GetColor(std::hash<std::string>{}(str));
  }
  [[nodiscard]] static Color GetThreadColor(uint32_t tid) {
    return GetColor(static_cast<uint32_t>(tid));
  }

  [[nodiscard]] uint64_t GetCaptureMin() const { return capture_min_timestamp_; }
  [[nodiscard]] uint64_t GetCaptureMax() const { return capture_max_timestamp_; }

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override;

  [[nodiscard]] AccessibleInterfaceProvider* GetAccessibleParent() const {
    return accessible_parent_;
  }

 protected:
  void PrepareBatcherAndUpdatePrimitives(PickingMode picking_mode);
  void DoUpdateLayout() override;
  void UpdateChildrenPosAndContainerSize();

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  void ProcessAsyncTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessSystemMemoryTrackingTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessCGroupAndProcessMemoryTrackingTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessPageFaultsTrackingTimer(const orbit_client_protos::TimerInfo& timer_info);

 private:
  AccessibleInterfaceProvider* accessible_parent_;
  TextRenderer text_renderer_static_;

  double ref_time_us_ = 0;
  double min_time_us_ = 0;
  double max_time_us_ = 0;
  uint64_t capture_min_timestamp_ = std::numeric_limits<uint64_t>::max();
  uint64_t capture_max_timestamp_ = 0;
  double time_window_us_ = 0;

  TimeGraphLayout layout_;

  bool update_primitives_requested_ = false;

  Batcher batcher_;

  std::unique_ptr<orbit_gl::TrackContainer> track_container_;
  std::unique_ptr<orbit_gl::TimelineUi> timeline_ui_;

  ManualInstrumentationManager* manual_instrumentation_manager_;
  orbit_client_data::ThreadTrackDataProvider* thread_track_data_provider_ = nullptr;
  const orbit_client_data::CaptureData* capture_data_ = nullptr;

  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_TIME_GRAPH_H_
