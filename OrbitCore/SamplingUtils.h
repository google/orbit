// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SAMPLING_UTILS_H_
#define ORBIT_CORE_SAMPLING_UTILS_H_

#include "SamplingProfiler.h"

namespace SamplingUtils {

[[nodiscard]] unsigned int GetCountForAddress(const ThreadSampleData& data,
                                              uint64_t address);

}  // namespace SamplingUtils

#endif  // ORBIT_CORE_SAMPLING_UTILS_H_
