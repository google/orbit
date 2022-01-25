// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_
#define CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_

#include "ClientData/CallstackData.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"

namespace orbit_client_model {
orbit_client_data::PostProcessedSamplingData CreatePostProcessedSamplingData(
    const orbit_client_data::CallstackData& callstack_data,
    const orbit_client_data::CaptureData& capture_data,
    const orbit_client_data::ModuleManager& module_manager, bool generate_summary = true);
}  // namespace orbit_client_model

#endif  // CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_
