// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Callstack.h"

#include "Capture.h"
#include "FunctionUtils.h"
#include "Serialization.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CallStack, 0) {
  ORBIT_NVP_VAL(0, m_Data);
  ORBIT_NVP_VAL(0, m_Hash);
}
