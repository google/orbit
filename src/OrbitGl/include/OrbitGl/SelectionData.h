
// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_SELECTION_DATA_H_
#define CLIENT_DATA_SELECTION_DATA_H_

#include <absl/types/span.h>

#include <memory>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/CallTreeView.h"

// This is meant to hold the data needed to update the data tabs to a specific selection.
class SelectionData {
 public:
  enum class SelectionType {
    kUnknown,
    kInspection,
  };

  // Delete move/copy operators because callstack_data_pointer_ is pointing to a variable inside the
  // class.
  SelectionData(const SelectionData& other) = delete;
  SelectionData& operator=(const SelectionData& other) = delete;
  SelectionData(SelectionData&& other) = delete;
  SelectionData& operator=(SelectionData&& other) = delete;

  SelectionData(const orbit_client_data::ModuleManager* module_manager,
                const orbit_client_data::CaptureData* capture_data,
                orbit_client_data::PostProcessedSamplingData post_processed_sampling_data,
                const orbit_client_data::CallstackData* callstack_data);

  SelectionData(const orbit_client_data::ModuleManager* module_manager,
                const orbit_client_data::CaptureData* capture_data,
                absl::Span<const orbit_client_data::CallstackEvent> callstack_events,
                SelectionType selection_type = SelectionType::kUnknown);

  std::shared_ptr<const CallTreeView> GetTopDownView() const { return top_down_view_; }

  std::shared_ptr<const CallTreeView> GetBottomUpView() const { return bottom_up_view_; }

  const orbit_client_data::PostProcessedSamplingData& GetPostProcessedSamplingData() const {
    return post_processed_sampling_data_;
  }

  const orbit_client_data::CallstackData& GetCallstackData() const {
    ORBIT_CHECK(callstack_data_pointer_);
    return *callstack_data_pointer_;
  }

  [[nodiscard]] bool IsInspection() const { return selection_type_ == SelectionType::kInspection; }

 private:
  std::shared_ptr<CallTreeView> top_down_view_;
  std::shared_ptr<CallTreeView> bottom_up_view_;

  orbit_client_data::PostProcessedSamplingData post_processed_sampling_data_;

  orbit_client_data::CallstackData callstack_data_object_;
  // Depending on how SelectionData is created, this either points to callstack_data_object_ or to
  // the CallstackData object passed into SelectionData.
  const orbit_client_data::CallstackData* callstack_data_pointer_ = &callstack_data_object_;

  SelectionType selection_type_ = SelectionType::kUnknown;
};

#endif  // CLIENT_DATA_SELECTION_DATA_H_