// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadStateBar.h"

#include <absl/strings/str_format.h>

#include <memory>
#include <utility>

#include "App.h"
#include "Batcher.h"
#include "CaptureViewElement.h"
#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "Viewport.h"

using orbit_client_data::ThreadID;
using orbit_client_protos::ThreadStateSliceInfo;

namespace orbit_gl {

ThreadStateBar::ThreadStateBar(CaptureViewElement* parent, OrbitApp* app,
                               const orbit_gl::TimelineInfoInterface* timeline_info,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                               const orbit_client_data::CaptureData* capture_data,
                               ThreadID thread_id, const Color& color)
    : ThreadBar(parent, app, timeline_info, viewport, layout, capture_data, thread_id,
                "ThreadState", color) {}

bool ThreadStateBar::IsEmpty() const {
  return capture_data_ == nullptr || !capture_data_->HasThreadStatesForThread(GetThreadId());
}

void ThreadStateBar::DoDraw(Batcher& batcher, TextRenderer& text_renderer,
                            const DrawContext& draw_context) {
  ThreadBar::DoDraw(batcher, text_renderer, draw_context);

  // Similarly to CallstackThreadBar::DoDraw, the thread state slices don't respond to clicks, but
  // have a tooltip. For picking, we want to draw the event bar over them if handling a click, and
  // underneath otherwise. This simulates "click-through" behavior.
  float thread_state_bar_z = draw_context.picking_mode == PickingMode::kClick
                                 ? GlCanvas::kZValueEventBarPicking
                                 : GlCanvas::kZValueEventBar;

  // Draw a transparent track just for clicking.
  Box box(GetPos(), Vec2(GetWidth(), GetHeight()), thread_state_bar_z);
  static const Color kTransparent{0, 0, 0, 0};
  batcher.AddBox(box, kTransparent, shared_from_this());
}

static Color GetThreadStateColor(ThreadStateSliceInfo::ThreadState state) {
  static const Color kGreen500{76, 175, 80, 255};
  static const Color kBlue500{33, 150, 243, 255};
  static const Color kGray600{117, 117, 117, 255};
  static const Color kOrange500{255, 152, 0, 255};
  static const Color kRed500{244, 67, 54, 255};
  static const Color kPurple500{156, 39, 176, 255};
  static const Color kBlack{0, 0, 0, 255};
  static const Color kBrown500{121, 85, 72, 255};

  switch (state) {
    case ThreadStateSliceInfo::kRunning:
      return kGreen500;
    case ThreadStateSliceInfo::kRunnable:
      return kBlue500;
    case ThreadStateSliceInfo::kInterruptibleSleep:
      return kGray600;
    case ThreadStateSliceInfo::kUninterruptibleSleep:
      return kOrange500;
    case ThreadStateSliceInfo::kStopped:
      return kRed500;
    case ThreadStateSliceInfo::kTraced:
      return kPurple500;
    case ThreadStateSliceInfo::kDead:
      [[fallthrough]];
    case ThreadStateSliceInfo::kZombie:
      return kBlack;
    case ThreadStateSliceInfo::kParked:
      [[fallthrough]];
    case ThreadStateSliceInfo::kIdle:
      return kBrown500;
    default:
      UNREACHABLE();
  }
}

static std::string GetThreadStateName(ThreadStateSliceInfo::ThreadState state) {
  switch (state) {
    case ThreadStateSliceInfo::kRunning:
      return "Running";
    case ThreadStateSliceInfo::kRunnable:
      return "Runnable";
    case ThreadStateSliceInfo::kInterruptibleSleep:
      return "Interruptible sleep";
    case ThreadStateSliceInfo::kUninterruptibleSleep:
      return "Uninterruptible sleep";
    case ThreadStateSliceInfo::kStopped:
      return "Stopped";
    case ThreadStateSliceInfo::kTraced:
      return "Traced";
    case ThreadStateSliceInfo::kDead:
      return "Dead";
    case ThreadStateSliceInfo::kZombie:
      return "Zombie";
    case ThreadStateSliceInfo::kParked:
      return "Parked";
    case ThreadStateSliceInfo::kIdle:
      return "Idle";
    default:
      UNREACHABLE();
  }
}

static std::string GetThreadStateDescription(ThreadStateSliceInfo::ThreadState state) {
  switch (state) {
    case ThreadStateSliceInfo::kRunning:
      return "The thread is currently scheduled on the CPU.";
    case ThreadStateSliceInfo::kRunnable:
      return "The thread is ready to use the CPU, but is currently not scheduled.";
    case ThreadStateSliceInfo::kInterruptibleSleep:
      return "The thread is waiting for a resource to become available or for an event to happen.";
    case ThreadStateSliceInfo::kUninterruptibleSleep:
      return "The thread performed a specific system call that cannot be interrupted by any signal "
             "and is waiting for the call to complete.";
    case ThreadStateSliceInfo::kStopped:
      return "The execution of the thread was suspended with the SIGSTOP signal.";
    case ThreadStateSliceInfo::kTraced:
      return "The thread is stopped because a tracer (for example, a debugger) is attached to it.";
    case ThreadStateSliceInfo::kDead:
      [[fallthrough]];
    case ThreadStateSliceInfo::kZombie:
      return "The thread has exited.";
    case ThreadStateSliceInfo::kParked:
      return "Parked kernel thread.";
    case ThreadStateSliceInfo::kIdle:
      return "Idle kernel thread.";
    default:
      UNREACHABLE();
  }
}

std::string ThreadStateBar::GetThreadStateSliceTooltip(Batcher& batcher, PickingId id) const {
  PickingUserData* user_data = batcher.GetUserData(id);
  if (user_data == nullptr || user_data->custom_data_ == nullptr) {
    return "";
  }

  const auto* thread_state_slice =
      static_cast<const ThreadStateSliceInfo*>(user_data->custom_data_);
  return absl::StrFormat(
      "<b>%s</b><br/>"
      "<i>Thread state</i><br/>"
      "<br/>"
      "%s",
      GetThreadStateName(thread_state_slice->thread_state()),
      GetThreadStateDescription(thread_state_slice->thread_state()));
}

void ThreadStateBar::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                        uint64_t min_tick, uint64_t max_tick,
                                        PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("ThreadStateBar::DoUpdatePrimitives", kOrbitColorTeal);
  ThreadBar::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);

  const auto time_window_ns = static_cast<uint64_t>(1000 * timeline_info_->GetTimeWindowUs());
  const uint64_t pixel_delta_ns = time_window_ns / viewport_->WorldToScreen(GetSize())[0];
  const uint64_t min_time_graph_ns = timeline_info_->GetTickFromUs(timeline_info_->GetMinTimeUs());
  const float pixel_width_in_world_coords = viewport_->ScreenToWorld({1, 0})[0];

  uint64_t ignore_until_ns = 0;

  CHECK(capture_data_ != nullptr);
  capture_data_->ForEachThreadStateSliceIntersectingTimeRange(
      GetThreadId(), min_tick, max_tick, [&](const ThreadStateSliceInfo& slice) {
        if (slice.end_timestamp_ns() <= ignore_until_ns) {
          // Reduce overdraw by not drawing slices whose entire width would only draw over a
          // previous slice. Similar to TimerTrack::UpdatePrimitives.
          return;
        }

        const float x0 = timeline_info_->GetWorldFromTick(slice.begin_timestamp_ns());
        const float x1 = timeline_info_->GetWorldFromTick(slice.end_timestamp_ns());
        const float width = x1 - x0;

        const Vec2 pos{x0, GetPos()[1]};
        const Vec2 size{width, GetHeight()};

        const Color color = GetThreadStateColor(slice.thread_state());

        auto user_data = std::make_unique<PickingUserData>(
            nullptr, [&](PickingId id) { return GetThreadStateSliceTooltip(batcher, id); });
        user_data->custom_data_ = &slice;

        if (slice.end_timestamp_ns() - slice.begin_timestamp_ns() > pixel_delta_ns) {
          Box box(pos, size, GlCanvas::kZValueEvent);
          batcher.AddBox(box, color, std::move(user_data));
        } else {
          // Make this slice cover an entire pixel and don't draw subsequent slices that would
          // coincide with the same pixel.
          // Use AddBox instead of AddVerticalLine as otherwise the tops of Boxes and lines wouldn't
          // be properly aligned.
          Box box(pos, {pixel_width_in_world_coords, size[1]}, GlCanvas::kZValueEvent);
          batcher.AddBox(box, color, std::move(user_data));

          if (pixel_delta_ns != 0) {
            ignore_until_ns =
                min_time_graph_ns +
                (slice.begin_timestamp_ns() - min_time_graph_ns) / pixel_delta_ns * pixel_delta_ns +
                pixel_delta_ns;
          }
        }
      });
}

void ThreadStateBar::OnPick(int x, int y) {
  ThreadBar::OnPick(x, y);
  app_->set_selected_thread_id(GetThreadId());
}

}  // namespace orbit_gl
