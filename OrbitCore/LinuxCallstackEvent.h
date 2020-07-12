// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#ifndef ORBIT_CORE_LINUX_CALLSTACK_EVENT_H_
#define ORBIT_CORE_LINUX_CALLSTACK_EVENT_H_

#include <cstdint>

#include "capture.pb.h"

struct LinuxCallstackEvent {
  LinuxCallstackEvent() = default;
  LinuxCallstackEvent(uint64_t time, Callstack callstack)
      : time_{time}, callstack_{std::move(callstack)} {}

  uint64_t time_ = 0;
  Callstack callstack_;
};

#endif  // ORBIT_CORE_LINUX_CALLSTACK_EVENT_H_
