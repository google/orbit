// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include <string>
#include <utility>

#include "Callstack.h"
#include "Serialization.h"
#include "SerializationMacros.h"

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
