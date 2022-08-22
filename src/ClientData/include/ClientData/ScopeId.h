// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_API_SCOPE_ID_H_
#define ORBIT_CLIENT_DATA_API_SCOPE_ID_H_

#include "OrbitBase/Typedef.h"

namespace orbit_client_data {

struct ScopeIdTag : orbit_base::PostIncrementTag {};

// The typedef is used for uniform treatment of events of various types. In particular,
// manual instrumentation events, and dynamic instrumentation events. In particular, this is used
// for computation and visualization of aggregated statistics (see
// go/stadia-orbit-manual-instrumentation-aggregation).
using ScopeId = orbit_base::Typedef<ScopeIdTag, uint64_t>;

static_assert(orbit_base::kHasZeroMemoryOverheadV<ScopeId>);

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_API_SCOPE_ID_H_