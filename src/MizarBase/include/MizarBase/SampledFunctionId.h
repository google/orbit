// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_SAMPLED_FUNCTION_ID_H_
#define MIZAR_BASE_SAMPLED_FUNCTION_ID_H_

#include <absl/strings/str_format.h>
#include <stdint.h>

#include <string>
#include <utility>

#include "OrbitBase/Typedef.h"

namespace orbit_mizar_base {

struct SampledFunctionIdTag {};

// The class represents a sampled function id. These ids are the same for the same function across
// all the captures.
// TODO (b/236358265) rename to `SampledFunctionId`.
using SFID = orbit_base::Typedef<SampledFunctionIdTag, const uint64_t>;

// Making sure we do not waste memory on the abstraction
static_assert(sizeof(SFID) == sizeof(uint64_t));

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_SAMPLED_FUNCTION_ID_H_
