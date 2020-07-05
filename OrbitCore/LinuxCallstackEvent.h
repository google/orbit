// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#ifndef ORBIT_CORE_LINUX_CALLSTACK_EVENT_H_
#define ORBIT_CORE_LINUX_CALLSTACK_EVENT_H_

#include <string>
#include <utility>

#include "Callstack.h"
#include "Serialization.h"

struct LinuxCallstackEvent {
  LinuxCallstackEvent() = default;
  LinuxCallstackEvent(uint64_t time, CallStack callstack)
      : time_{time}, callstack_{std::move(callstack)} {}

  uint64_t time_ = 0;
  CallStack callstack_;

  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(LinuxCallstackEvent, 1) {
  ORBIT_NVP_VAL(1, time_);
  ORBIT_NVP_VAL(1, callstack_);
}

#endif // ORBIT_CORE_LINUX_CALLSTACK_EVENT_H_
