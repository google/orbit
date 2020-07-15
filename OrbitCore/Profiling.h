// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "BaseTypes.h"
#include "Platform.h"
#include "Utils.h"
#include "absl/time/time.h"

//-----------------------------------------------------------------------------
typedef uint64_t TickType;

//-----------------------------------------------------------------------------
#ifdef _WIN32
inline void clock_gettime(uint32_t, struct timespec* spec) {
  __int64 time;
  GetSystemTimeAsFileTime((FILETIME*)&time);
  spec->tv_sec = time / 10000000i64;
  spec->tv_nsec = time % 10000000i64 * 100;
}
#endif

//-----------------------------------------------------------------------------
inline TickType OrbitTicks(uint32_t a_Clock = 1 /*CLOCK_MONOTONIC*/) {
  timespec ts;
  clock_gettime(a_Clock, &ts);
  return 1000000000ll * ts.tv_sec + ts.tv_nsec;
}

//-----------------------------------------------------------------------------
inline double MicroSecondsFromTicks(TickType a_Start, TickType a_End) {
  return double((a_End - a_Start)) * 0.001;
}

inline double MilliSecondsFromTicks(TickType a_Start, TickType a_End) {
  absl::Duration micros = absl::Microseconds(MicroSecondsFromTicks(a_Start, a_End));
  return micros / absl::Milliseconds(1);
}

//-----------------------------------------------------------------------------
inline TickType TicksFromMicroseconds(double a_Micros) {
  return static_cast<TickType>(a_Micros * 1000);
}
