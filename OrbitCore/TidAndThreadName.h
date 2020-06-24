// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TID_AND_THREAD_NAME_H_
#define ORBIT_CORE_TID_AND_THREAD_NAME_H_

#include <string>
#include <utility>

#include "Serialization.h"
#include "SerializationMacros.h"

struct TidAndThreadName {
  TidAndThreadName() = default;
  TidAndThreadName(int32_t tid, std::string thread_name)
      : tid{tid}, thread_name{std::move(thread_name)} {}

  int32_t tid = 0;
  std::string thread_name;

  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(TidAndThreadName, 0) {
  ORBIT_NVP_VAL(0, tid);
  ORBIT_NVP_VAL(0, thread_name);
}

#endif  // ORBIT_CORE_TID_AND_THREAD_NAME_H_
