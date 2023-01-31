
// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SelectionData.h"

#include <utility>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientModel/SamplingDataPostProcessor.h"

using orbit_client_data::CallstackData;
using orbit_client_data::CallstackEvent;
using orbit_client_data::CaptureData;
using orbit_client_data::ModuleManager;
using orbit_client_data::PostProcessedSamplingData;

SelectionData::SelectionData(const ModuleManager* module_manager, const CaptureData* capture_data,
                             PostProcessedSamplingData post_processed_sampling_data,
                             const CallstackData* callstack_data)
    : post_processed_sampling_data_(std::move(post_processed_sampling_data)),
      callstack_data_pointer_(callstack_data) {
  top_down_view_ = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      post_processed_sampling_data_, module_manager, capture_data);
  bottom_up_view_ = CallTreeView::CreateBottomUpViewFromPostProcessedSamplingData(
      post_processed_sampling_data_, module_manager, capture_data);
}

SelectionData::SelectionData(const ModuleManager* module_manager, const CaptureData* capture_data,
                             absl::Span<const CallstackEvent> callstack_events,
                             SelectionType selection_type) {
  for (const CallstackEvent& event : callstack_events) {
    callstack_data_object_.AddCallstackFromKnownCallstackData(event,
                                                              capture_data->GetCallstackData());
  }
  post_processed_sampling_data_ = orbit_client_model::CreatePostProcessedSamplingData(
      callstack_data_object_, *capture_data, *module_manager);
  top_down_view_ = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      post_processed_sampling_data_, module_manager, capture_data);
  bottom_up_view_ = CallTreeView::CreateBottomUpViewFromPostProcessedSamplingData(
      post_processed_sampling_data_, module_manager, capture_data);
  selection_type_ = selection_type;
}