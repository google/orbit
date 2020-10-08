// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadStateTrack.h"

#include "App.h"
#include "capture_data.pb.h"

using orbit_client_protos::ThreadStateSliceInfo;

ThreadStateTrack::ThreadStateTrack(TimeGraph* time_graph, int32_t thread_id) : Track{time_graph} {
  thread_id_ = thread_id;
  picked_ = false;
}

bool ThreadStateTrack::IsEmpty() const {
  return !GOrbitApp->GetCaptureData().HasThreadStatesForThread(thread_id_);
}

void ThreadStateTrack::Draw(GlCanvas* canvas, PickingMode picking_mode) {
  // Similarly to EventTrack::Draw, the thread state slices don't respond to clicks, but have a
  // tooltip. For picking, we want to draw the event bar over them if handling a click, and
  // underneath otherwise. This simulates "click-through" behavior.
  const float thread_state_bar_z = picking_mode == PickingMode::kClick
                                       ? GlCanvas::kZValueEventBarPicking
                                       : GlCanvas::kZValueEventBar;

  // Draw a transparent track just for clicking.
  Batcher* batcher = canvas->GetBatcher();
  Box box(pos_, Vec2(size_[0], -size_[1]), thread_state_bar_z);
  static const Color kTransparent{0, 0, 0, 0};
  batcher->AddBox(box, kTransparent, shared_from_this());
}

static Color GetThreadStateColor(ThreadStateSliceInfo::ThreadState state) {
  static const Color kGreen{0, 224, 0, 255};
  static const Color kLightBlue{0, 255, 255, 255};
  static const Color kYellow{255, 192, 0, 255};
  static const Color kOrange{255, 128, 0, 255};
  static const Color kRed{255, 0, 0, 255};
  static const Color kGrey{128, 128, 128, 255};
  static const Color kBlack{0, 0, 0, 255};

  switch (state) {
    case ThreadStateSliceInfo::kRunning:
      return kGreen;
    case ThreadStateSliceInfo::kRunnable:
      return kLightBlue;
    case ThreadStateSliceInfo::kInterruptibleSleep:
      return kYellow;
    case ThreadStateSliceInfo::kUninterruptibleSleep:
      return kOrange;
    case ThreadStateSliceInfo::kStopped:
    case ThreadStateSliceInfo::kTraced:
      return kRed;
    case ThreadStateSliceInfo::kDead:
    case ThreadStateSliceInfo::kZombie:
      return kBlack;
    case ThreadStateSliceInfo::kParked:
    case ThreadStateSliceInfo::kIdle:
      return kGrey;
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

std::string ThreadStateTrack::GetThreadStateSliceTooltip(PickingId id) const {
  auto user_data = time_graph_->GetBatcher().GetUserData(id);
  if (user_data == nullptr || user_data->custom_data_ == nullptr) {
    return "";
  }

  const auto* thread_state_slice =
      static_cast<const ThreadStateSliceInfo*>(user_data->custom_data_);
  return absl::StrFormat(
      "<b>%s</b><br/>"
      "<i>Thread state</i><br/>",
      GetThreadStateName(thread_state_slice->thread_state()));
}

void ThreadStateTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick,
                                        PickingMode /*picking_mode*/) {
  Batcher* batcher = &time_graph_->GetBatcher();
  GOrbitApp->GetCaptureData().ForEachThreadStateSliceIntersectingTimeRange(
      thread_id_, min_tick, max_tick, [&](const ThreadStateSliceInfo& slice) {
        float x0 = std::max(pos_[0], time_graph_->GetWorldFromTick(slice.begin_timestamp_ns()));
        float x1 =
            std::min(pos_[0] + size_[0], time_graph_->GetWorldFromTick(slice.end_timestamp_ns()));
        Vec2 pos{x0, pos_[1]};
        Vec2 size{x1 - x0, -size_[1]};
        Box box(pos, size, GlCanvas::kZValueEvent);

        auto user_data = std::make_unique<PickingUserData>(
            nullptr, [&](PickingId id) { return GetThreadStateSliceTooltip(id); });
        user_data->custom_data_ = &slice;
        batcher->AddBox(box, GetThreadStateColor(slice.thread_state()), std::move(user_data));
      });
}

void ThreadStateTrack::OnPick(int /*x*/, int /*y*/) {
  GOrbitApp->set_selected_thread_id(thread_id_);
  picked_ = true;
}
