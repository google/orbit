// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/SafeStrerror.h"

#include <array>
#include <cstring>

const char* SafeStrerror(int errnum) {
  thread_local std::array<char, 256> buf;
#ifdef _MSC_VER
  strerror_s(buf.data(), buf.size(), errnum);
  return buf.data();
#else
  return strerror_r(errnum, buf.data(), buf.size());
#endif
}
