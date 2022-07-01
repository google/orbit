// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_THREAD_ID_H_
#define MIZAR_BASE_THREAD_ID_H_

#include <stdint.h>

#include "OrbitBase/Typedef.h"

namespace orbit_mizar_base {

struct TIDTag {};

using TID = orbit_base::Typedef<TIDTag, const uint32_t>;

// Making sure we do not waste memory on the abstraction
static_assert(sizeof(TID) == sizeof(uint32_t));

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_THREAD_ID_H_
