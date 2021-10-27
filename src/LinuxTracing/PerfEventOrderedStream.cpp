// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEventOrderedStream.h"

namespace orbit_linux_tracing {

const PerfEventOrderedStream PerfEventOrderedStream::kNone{OrderType::kNotOrdered, 0};

}  // namespace orbit_linux_tracing
