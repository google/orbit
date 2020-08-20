// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CAPTURE_H_
#define ORBIT_CORE_CAPTURE_H_

#include <chrono>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"

class Capture {
 public:
  static CaptureData capture_data_;
};

#endif  // ORBIT_CORE_CAPTURE_H_
