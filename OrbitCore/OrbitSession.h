// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_ORBIT_SESSION_H_
#define ORBIT_CORE_ORBIT_SESSION_H_

#include "preset.pb.h"

struct Preset {
  std::string file_name;
  orbit_client_protos::PresetInfo preset_info;
};

#endif  // ORBIT_CORE_ORBIT_SESSION_H_
