// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/LiveFunctionsController.h"

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/meta/type_traits.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include "ClientData/CaptureData.h"
#include "ClientData/ScopeId.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TrackContainer.h"

using orbit_client_data::FunctionInfo;
using orbit_client_data::ScopeId;
using orbit_client_protos::TimerInfo;

namespace {

std::pair<uint64_t, uint64_t> ComputeMinMaxTime(
    const absl::flat_hash_map<uint64_t, const TimerInfo*>& timer_infos) {
  uint64_t min_time = std::numeric_limits<uint64_t>::max();
  uint64_t max_time = std::numeric_limits<uint64_t>::min();
  for (const auto& timer_info : timer_infos) {
    min_time = std::min(min_time, timer_info.second->start());
    max_time = std::max(max_time, timer_info.second->start());
  }
  return std::make_pair(min_time, max_time);
}

uint64_t AbsDiff(uint64_t a, uint64_t b) {
  if (a > b) {
    return a - b;
  }
  return b - a;
}

const orbit_client_protos::TimerInfo* ClosestTo(uint64_t point,
                                                const orbit_client_protos::TimerInfo* timer_a,
                                                const orbit_client_protos::TimerInfo* timer_b) {
  uint64_t a_diff = AbsDiff(point, timer_a->start());
  uint64_t b_diff = AbsDiff(point, timer_b->start());
  if (a_diff <= b_diff) {
    return timer_a;
  }
  return timer_b;
}

static const orbit_client_protos::TimerInfo* SnapToClosestStart(TimeGraph* time_graph,
                                                                ScopeId scope_id) {
  double min_us = time_graph->GetMinTimeUs();
  double max_us = time_graph->GetMaxTimeUs();
  double center_us = 0.5 * max_us + 0.5 * min_us;
  uint64_t center = time_graph->GetTickFromUs(center_us);

  // First, we find the next function call (text box) that has its end timestamp
  // after center - 1 (we use center - 1 to make sure that center itself is
  // included in the timerange that we search). Note that FindNextFunctionCall
  // uses the end marker of the timer as a timestamp.
  const orbit_client_protos::TimerInfo* timer_info =
      time_graph->FindNextScopeTimer(scope_id, center - 1);

  // If we cannot find a next function call, then the closest one is the first
  // call we find before center.
  if (!timer_info) {
    return time_graph->FindPreviousScopeTimer(scope_id, center);
  }

  // We have to consider the case where center falls to the right of the start
  // marker of 'box'. In this case, the closest box can be any of two boxes:
  // 'box' or the next one. It cannot be any box before 'box' because we are
  // using the start marker to measure the distance.
  if (timer_info->start() <= center) {
    const orbit_client_protos::TimerInfo* next_timer_info =
        time_graph->FindNextScopeTimer(scope_id, timer_info->end());
    if (!next_timer_info) {
      return timer_info;
    }

    return ClosestTo(center, timer_info, next_timer_info);
  }

  // The center is to the left of 'box', so the closest box is either 'box' or
  // the next box to the left of the center.
  const orbit_client_protos::TimerInfo* previous_timer_info =
      time_graph->FindPreviousScopeTimer(scope_id, timer_info->start());

  if (!previous_timer_info) {
    return timer_info;
  }

  return ClosestTo(center, previous_timer_info, timer_info);
}

}  // namespace

LiveFunctionsController::LiveFunctionsController(OrbitApp* app)
    : live_functions_data_view_{this, app}, app_{app} {
  live_functions_data_view_.Init();
}

void LiveFunctionsController::Move() {
  if (!current_timer_infos_.empty()) {
    auto min_max = ComputeMinMaxTime(current_timer_infos_);
    app_->GetMutableTimeGraph()->HorizontallyMoveIntoView(TimeGraph::VisibilityType::kFullyVisible,
                                                          min_max.first, min_max.second, 0.5);
  }
  app_->GetMutableTimeGraph()->GetTrackContainer()->SetIteratorOverlayData(
      current_timer_infos_, iterator_id_to_scope_id_);
}

bool LiveFunctionsController::OnAllNextButton() {
  absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*> next_timer_infos;
  uint64_t id_with_min_timestamp = 0;
  uint64_t min_timestamp = std::numeric_limits<uint64_t>::max();
  for (auto it : iterator_id_to_scope_id_) {
    ScopeId scope_id = it.second;
    const orbit_client_protos::TimerInfo* current_timer_info =
        current_timer_infos_.find(it.first)->second;
    const orbit_client_protos::TimerInfo* timer_info =
        app_->GetMutableTimeGraph()->FindNextScopeTimer(scope_id, current_timer_info->end());
    if (timer_info == nullptr) {
      return false;
    }
    if (timer_info->start() < min_timestamp) {
      min_timestamp = timer_info->start();
      id_with_min_timestamp = it.first;
    }
    next_timer_infos.insert(std::make_pair(it.first, timer_info));
  }

  // We only want to commit to the new boxes when all boxes can be moved.
  current_timer_infos_ = next_timer_infos;
  id_to_select_ = id_with_min_timestamp;
  Move();
  return true;
}

bool LiveFunctionsController::OnAllPreviousButton() {
  absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*> next_timer_infos;
  uint64_t id_with_min_timestamp = 0;
  uint64_t min_timestamp = std::numeric_limits<uint64_t>::max();
  for (auto it : iterator_id_to_scope_id_) {
    ScopeId function_scope_id = it.second;
    const orbit_client_protos::TimerInfo* current_timer_info =
        current_timer_infos_.find(it.first)->second;
    const orbit_client_protos::TimerInfo* timer_info =
        app_->GetMutableTimeGraph()->FindPreviousScopeTimer(function_scope_id,
                                                            current_timer_info->end());
    if (timer_info == nullptr) {
      return false;
    }
    if (timer_info->start() < min_timestamp) {
      min_timestamp = timer_info->start();
      id_with_min_timestamp = it.first;
    }
    next_timer_infos.insert(std::make_pair(it.first, timer_info));
  }

  // We only want to commit to the new boxes when all boxes can be moved.
  current_timer_infos_ = next_timer_infos;
  id_to_select_ = id_with_min_timestamp;
  Move();
  return true;
}

void LiveFunctionsController::OnNextButton(uint64_t id) {
  const orbit_client_protos::TimerInfo* timer_info =
      app_->GetMutableTimeGraph()->FindNextScopeTimer(iterator_id_to_scope_id_[id],
                                                      current_timer_infos_[id]->end());
  // If text_box is nullptr, then we have reached the right end of the timeline.
  if (timer_info != nullptr) {
    current_timer_infos_[id] = timer_info;
  }
  id_to_select_ = id;
  Move();
}
void LiveFunctionsController::OnPreviousButton(uint64_t id) {
  const orbit_client_protos::TimerInfo* timer_info =
      app_->GetMutableTimeGraph()->FindPreviousScopeTimer(iterator_id_to_scope_id_[id],
                                                          current_timer_infos_[id]->end());
  // If text_box is nullptr, then we have reached the left end of the timeline.
  if (timer_info != nullptr) {
    current_timer_infos_[id] = timer_info;
  }
  id_to_select_ = id;
  Move();
}

void LiveFunctionsController::OnDeleteButton(uint64_t id) {
  current_timer_infos_.erase(id);
  iterator_id_to_scope_id_.erase(id);

  // If we erase the iterator that was last used by the user, then
  // we need to switch last_id_pressed_ to an existing id.
  if (id == id_to_select_ && !current_timer_infos_.empty()) {
    id_to_select_ = current_timer_infos_.begin()->first;
  } else if (current_timer_infos_.empty()) {
    id_to_select_ = orbit_grpc_protos::kInvalidFunctionId;
  }
  Move();
}

void LiveFunctionsController::AddIterator(ScopeId instrumented_function_scope_id,
                                          const FunctionInfo* function) {
  uint64_t iterator_id = next_iterator_id_++;
  const orbit_client_protos::TimerInfo* timer_info = app_->selected_timer();
  // If no box is currently selected or the selected box is a different
  // function, we search for the closest box to the current center of the
  // screen.
  if (!timer_info ||
      FunctionIdToScopeId(timer_info->function_id()) != instrumented_function_scope_id) {
    timer_info = SnapToClosestStart(app_->GetMutableTimeGraph(), instrumented_function_scope_id);
  }

  iterator_id_to_scope_id_.insert(std::make_pair(iterator_id, instrumented_function_scope_id));
  current_timer_infos_.insert(std::make_pair(iterator_id, timer_info));
  id_to_select_ = iterator_id;
  if (add_iterator_callback_) {
    add_iterator_callback_(iterator_id, function);
  }
  Move();
}

uint64_t LiveFunctionsController::GetStartTime(uint64_t index) const {
  const auto& it = current_timer_infos_.find(index);
  if (it != current_timer_infos_.end()) {
    return it->second->start();
  }
  return GetCaptureMin();
}

uint64_t LiveFunctionsController::GetCaptureMin() const {
  ORBIT_CHECK(app_ != nullptr);
  return app_->GetTimeGraph()->GetCaptureMin();
}
uint64_t LiveFunctionsController::GetCaptureMax() const {
  ORBIT_CHECK(app_ != nullptr);
  return app_->GetTimeGraph()->GetCaptureMax();
}

void LiveFunctionsController::Reset() {
  iterator_id_to_scope_id_.clear();
  current_timer_infos_.clear();
  TimeGraph* time_graph = app_->GetMutableTimeGraph();
  if (time_graph != nullptr) {
    app_->GetMutableTimeGraph()->GetTrackContainer()->SetIteratorOverlayData({}, {});
  }
  id_to_select_ = orbit_grpc_protos::kInvalidFunctionId;
}

std::optional<ScopeId> LiveFunctionsController::FunctionIdToScopeId(uint64_t function_id) const {
  ORBIT_CHECK(app_ != nullptr);
  ORBIT_CHECK(app_->HasCaptureData());
  return app_->GetCaptureData().FunctionIdToScopeId(function_id);
}

void LiveFunctionsController::SetScopeStatsCollection(
    std::shared_ptr<const orbit_client_data::ScopeStatsCollection> scope_collection) {
  live_functions_data_view_.SetScopeStatsCollection(std::move(scope_collection));
}