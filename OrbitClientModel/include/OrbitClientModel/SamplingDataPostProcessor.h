// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_
#define ORBIT_CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_

#include "OrbitClientData/CallstackData.h"
#include "OrbitClientModel/CaptureData.h"

namespace orbit_client_model {
PostProcessedSamplingData CreatePostProcessedSamplingData(const CallstackData& callstack_data,
                                                          const CaptureData& capture_data,
                                                          bool generate_summary = true);
}  // namespace orbit_client_model

#endif  // ORBIT_CLIENT_MODEL_SAMPLING_DATA_POST_PROCESSOR_H_
