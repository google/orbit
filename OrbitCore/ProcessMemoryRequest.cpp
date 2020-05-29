// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessMemoryRequest.h"

#include "Serialization.h"

ORBIT_SERIALIZE(ProcessMemoryRequest, 0) {
  ORBIT_NVP_VAL(0, pid);
  ORBIT_NVP_VAL(0, address);
  ORBIT_NVP_VAL(0, size);
}
