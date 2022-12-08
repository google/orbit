// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_H_
#define ORBIT_GL_TIME_GRAPH_H_

#include <QPainter>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/ApiStringEvent.h"
#include "ClientData/ApiTrackValue.h"
#include "ClientData/CaptureData.h"
#include "ClientData/CgroupAndProcessMemoryInfo.h"
#include "ClientData/PageFaultsInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/SystemMemoryInfo.h"
#include "ClientData/ThreadTrackDataProvider.h"
#include "ClientData/TimerChain.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/AccessibleInterfaceProvider.h"
#include "OrbitGl/Batcher.h"
#include "OrbitGl/Button.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/GlSlider.h"
#include "OrbitGl/ManualInstrumentationManager.h"
#include "OrbitGl/OpenGlBatcher.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/QtTextRenderer.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/TimelineUi.h"
#include "OrbitGl/TrackContainer.h"
#include "OrbitGl/TrackManager.h"
#include "OrbitGl/Viewport.h"

class OrbitApp;

class TimeGraph : public orbit_gl::CaptureViewElement, public orbit_gl::TimelineInfoInterface {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  explicit TimeGraph(AccessibleInterfaceProvider* parent, OrbitApp* app,
                     orbit_gl::Viewport* viewport, orbit_client_data::CaptureData* capture_data,
                     PickingManager* picking_manager, TimeGraphLayout* time_graph_layout);

  [[nodiscard]] float GetHeight() const override;

  void DrawAllElements(orbit_gl::PrimitiveAssembler& primitive_assembler,
                       orbit_gl::TextRenderer& text_renderer, PickingMode& picking_mode);
  void DrawText(QPainter* painter, float layer);

  // TODO(b/214282122): Move Process Timers function outside the UI.
  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessCgroupAndProcessMemoryInfo(
      const orbit_client_data::CgroupAndProcessMemoryInfo& cgroup_and_process_memory_info);
  void ProcessPageFaultsInfo(const orbit_client_data::PageFaultsInfo& page_faults_info);
  void ProcessSystemMemoryInfo(const orbit_client_data::SystemMemoryInfo& system_memory_info);
  void ProcessApiStringEvent(const orbit_client_data::ApiStringEvent& string_event);
  void ProcessApiTrackValueEvent(const orbit_client_data::ApiTrackValue& track_event) const;

  [[nodiscard]] const orbit_client_data::CaptureData* GetCaptureData() const {
    return capture_data_;
  }
  [[nodiscard]] orbit_gl::TrackContainer* GetTrackContainer() const {
    return track_container_.get();
  }
  [[nodiscard]] orbit_gl::GlSlider* GetHorizontalSlider() const { return horizontal_slider_.get(); }
  [[nodiscard]] orbit_gl::GlSlider* GetVerticalSlider() const { return vertical_slider_.get(); }
  [[nodiscard]] orbit_gl::TimelineUi* GetTimelineUi() const { return timeline_ui_.get(); }
  [[nodiscard]] orbit_gl::Button* GetPlusButton() const { return plus_button_.get(); }
  [[nodiscard]] orbit_gl::Button* GetMinusButton() const { return minus_button_.get(); }
  [[nodiscard]] orbit_gl::TrackManager* GetTrackManager() const {
    return track_container_->GetTrackManager();
  }
  [[nodiscard]] Vec2 GetTimelinePos() const { return GetTimelineUi()->GetPos(); }
  [[nodiscard]] float GetTimelineWidth() const { return GetTimelineUi()->GetWidth(); }

  [[nodiscard]] float GetWorldFromTick(uint64_t time) const override;
  [[nodiscard]] float GetWorldFromUs(double micros) const override;
  [[nodiscard]] uint64_t GetTickFromWorld(float world_x) const override;
  [[nodiscard]] uint64_t GetTickFromUs(double micros) const override;
  [[nodiscard]] double GetUsFromTick(uint64_t time) const override;
  [[nodiscard]] uint64_t GetNsSinceStart(uint64_t time) const override;
  [[nodiscard]] double GetTimeWindowUs() const override { return max_time_us_ - min_time_us_; }
  [[nodiscard]] double GetMinTimeUs() const override { return min_time_us_; }
  [[nodiscard]] double GetMaxTimeUs() const override { return max_time_us_; }
  [[nodiscard]] uint64_t GetCaptureTimeSpanNs() const override;
  [[nodiscard]] std::pair<float, float> GetBoxPosXAndWidthFromTicks(
      uint64_t start_tick, uint64_t end_tick) const override;

  void UpdateCaptureMinMaxTimestamps();

  void ZoomAll();
  void Zoom(const orbit_client_protos::TimerInfo& timer_info);
  void Zoom(uint64_t min, uint64_t max);
  void ZoomTime(int zoom_delta, double center_time_ratio) override;
  void SetMinMax(double min_time_us, double max_time_us);
  void PanTime(int initial_x, int current_x, int width, double initial_time);
  void VerticalZoom(float zoom_value, float mouse_world_y_pos);

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

  enum class JumpScope { kSameDepth, kSameThread, kSameFunction, kSameThreadSameFunction };
  enum class JumpDirection { kPrevious, kNext, kTop, kDown };
  void JumpToNeighborTimer(const orbit_client_protos::TimerInfo* from, JumpDirection jump_direction,
                           JumpScope jump_scope);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindPreviousScopeTimer(
      ScopeId scope_id, uint64_t current_time,
      std::optional<uint32_t> thread_id = std::nullopt) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindNextScopeTimer(
      ScopeId scope_id, uint64_t current_time,
      std::optional<uint32_t> thread_id = std::nullopt) const;
  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetAllThreadTrackTimerChains()
      const;
  [[nodiscard]] std::pair<const orbit_client_protos::TimerInfo*,
                          const orbit_client_protos::TimerInfo*>
  GetMinMaxTimerForScope(ScopeId scope_id) const;

  void SelectAndZoom(const orbit_client_protos::TimerInfo* timer_info);
  [[nodiscard]] double GetCaptureTimeSpanUs() const;
  [[nodiscard]] bool IsRedrawNeeded() const {
    return draw_requested_ || update_primitives_requested_;
  }

  [[nodiscard]] orbit_gl::TextRenderer* GetTextRenderer() { return &text_renderer_static_; }
  [[nodiscard]] orbit_gl::Batcher& GetBatcher() { return batcher_; }

  [[nodiscard]] const TimeGraphLayout& GetLayout() const { return *layout_; }

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

  [[nodiscard]] static Color GetColor(std::string_view str) {
    return GetColor(std::hash<std::string_view>{}(str));
  }

  [[nodiscard]] uint64_t GetCaptureMin() const { return capture_min_timestamp_; }
  [[nodiscard]] uint64_t GetCaptureMax() const { return capture_max_timestamp_; }

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override;

  [[nodiscard]] AccessibleInterfaceProvider* GetAccessibleParent() const {
    return accessible_parent_;
  }

 protected:
  static constexpr double kTimeGraphMinTimeWindowsUs = 0.1; /* 100 ns */

  void DoDraw(orbit_gl::PrimitiveAssembler& primitive_assembler,
              orbit_gl::TextRenderer& text_renderer, const DrawContext& draw_context) override;
  void PrepareBatcherAndUpdatePrimitives(PickingMode picking_mode);
  void DoUpdateLayout() override;
  void UpdateChildrenPosAndContainerSize();
  void UpdateVerticalSliderFromWorld();
  void UpdateHorizontalSliderFromWorld();
  [[nodiscard]] float GetRightMargin() const;

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  void ProcessAsyncTimer(const orbit_client_protos::TimerInfo& timer_info) const;

  std::shared_ptr<orbit_gl::GlSlider> horizontal_slider_;
  std::shared_ptr<orbit_gl::GlSlider> vertical_slider_;

 private:
  [[nodiscard]] EventResult OnMouseWheel(
      const Vec2& mouse_pos, int delta,
      const orbit_gl::ModifierKeys& modifiers = orbit_gl::ModifierKeys()) override;
  [[nodiscard]] EventResult OnMouseLeave() override;

  void UpdateHorizontalScroll(float ratio);
  void UpdateHorizontalZoom(float normalized_start, float normalized_end);

  void SelectAndMakeVisible(const orbit_client_protos::TimerInfo* timer_info);
  [[nodiscard]] bool IsFullyVisible(uint64_t min, uint64_t max) const;
  [[nodiscard]] bool IsPartlyVisible(uint64_t min, uint64_t max) const;
  [[nodiscard]] bool IsVisible(VisibilityType vis_type, uint64_t min, uint64_t max) const;

  void DrawMarginsBetweenChildren(orbit_gl::PrimitiveAssembler& primitive_assembler) const;

  [[nodiscard]] const TimerInfo* FindNextThreadTrackTimer(ScopeId scope_id, uint64_t current_time,
                                                          std::optional<uint32_t> thread_id) const;

  [[nodiscard]] const TimerInfo* FindPreviousThreadTrackTimer(
      ScopeId scope_id, uint64_t current_time, std::optional<uint32_t> thread_id) const;

  std::pair<const TimerInfo*, const TimerInfo*> GetMinMaxTimerForThreadTrackScope(
      ScopeId scope_id) const;

  AccessibleInterfaceProvider* accessible_parent_;
  orbit_gl::QtTextRenderer text_renderer_static_;

  double ref_time_us_ = 0;
  double min_time_us_ = 0;
  double max_time_us_ = 0;
  uint64_t capture_min_timestamp_ = std::numeric_limits<uint64_t>::max();
  uint64_t capture_max_timestamp_ = 0;

  TimeGraphLayout* layout_ = nullptr;

  orbit_gl::OpenGlBatcher batcher_;
  orbit_gl::PrimitiveAssembler primitive_assembler_;

  std::unique_ptr<orbit_gl::TrackContainer> track_container_;
  std::unique_ptr<orbit_gl::TimelineUi> timeline_ui_;
  // A shared ptr is needed for the buttons because they are pickable.
  std::shared_ptr<orbit_gl::Button> plus_button_;
  std::shared_ptr<orbit_gl::Button> minus_button_;

  ManualInstrumentationManager* manual_instrumentation_manager_;
  orbit_client_data::ThreadTrackDataProvider* thread_track_data_provider_ = nullptr;
  const orbit_client_data::CaptureData* capture_data_ = nullptr;

  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_TIME_GRAPH_H_
