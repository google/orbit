// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_STUBS_H_
#define ORBIT_API_STUBS_H_

#include <stdint.h>

namespace orbit_api {

void Start(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
void Stop(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
void StartAsync(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
void StopAsync(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
void TrackValue(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

}  // namespace orbit_api

#endif
