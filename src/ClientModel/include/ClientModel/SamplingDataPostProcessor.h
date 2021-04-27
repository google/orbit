// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_
#define CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_

#include "ClientModel/CaptureData.h"
#include "OrbitClientData/CallstackData.h"
#include "OrbitClientData/PostProcessedSamplingData.h"

namespace orbit_client_model {
PostProcessedSamplingData CreatePostProcessedSamplingData(const CallstackData& callstack_data,
                                                          const CaptureData& capture_data,
                                                          bool generate_summary = true);
}  // namespace orbit_client_model

#endif  // CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_
