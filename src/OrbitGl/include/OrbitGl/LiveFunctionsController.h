// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIVE_FUNCTIONS_H_
#define LIVE_FUNCTIONS_H_

#include <absl/container/flat_hash_map.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStatsCollection.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/LiveFunctionsDataView.h"
#include "DataViews/LiveFunctionsInterface.h"
#include "absl/container/flat_hash_map.h"

class OrbitApp;

class LiveFunctionsController : public orbit_data_views::LiveFunctionsInterface {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  explicit LiveFunctionsController(OrbitApp* app);

  orbit_data_views::LiveFunctionsDataView& GetDataView() { return live_functions_data_view_; }

  bool OnAllNextButton();
  bool OnAllPreviousButton();

  void OnNextButton(uint64_t id);
  void OnPreviousButton(uint64_t id);
  void OnDeleteButton(uint64_t id);

  void Reset();
  void OnDataChanged() { live_functions_data_view_.OnDataChanged(); }

  void SetAddIteratorCallback(
      std::function<void(uint64_t, const orbit_client_data::FunctionInfo*)> callback) {
    add_iterator_callback_ = std::move(callback);
  }

  void SetScopeStatsCollection(
      std::shared_ptr<const orbit_client_data::ScopeStatsCollection> scope_collection);

  [[nodiscard]] uint64_t GetCaptureMin() const;
  [[nodiscard]] uint64_t GetCaptureMax() const;
  [[nodiscard]] uint64_t GetStartTime(uint64_t index) const;

  void AddIterator(ScopeId instrumented_function_scope_id,
                   const orbit_client_data::FunctionInfo* function) override;

 private:
  void Move();

  [[nodiscard]] std::optional<ScopeId> FunctionIdToScopeId(uint64_t function_id) const;

  orbit_data_views::LiveFunctionsDataView live_functions_data_view_;

  absl::flat_hash_map<uint64_t, ScopeId> iterator_id_to_scope_id_;
  absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*> current_timer_infos_;

  std::function<void(uint64_t, const orbit_client_data::FunctionInfo*)> add_iterator_callback_;

  uint64_t next_iterator_id_ = 1;
  uint64_t id_to_select_ = 0;

  OrbitApp* app_ = nullptr;
};

#endif  // LIVE_FUNCTIONS_H_
