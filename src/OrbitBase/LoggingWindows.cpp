// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Windows.h>

#include "OrbitBase/StringConversion.h"

namespace orbit_base {

void OutputToDebugger(const char* str) {
  std::wstring str_w = orbit_base::ToStdWString(str);
  ::OutputDebugStringW(str_w.c_str());
}
}  // namespace orbit_base