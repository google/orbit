// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_ORBIT_API_VERSIONS_H_
#define ORBIT_API_ORBIT_API_VERSIONS_H_

#include <stdint.h>

// This file contains versions of `struct orbit_api_..._v#` different from the one defined in
// Orbit.h.

// In particular, these are the versions older than the current one.

struct orbit_api_v0 {
  uint32_t enabled;
  uint32_t initialized;
  void (*start)(const char* name, orbit_api_color color);
  void (*stop)();
  void (*start_async)(const char* name, uint64_t id, orbit_api_color color);
  void (*stop_async)(uint64_t id);
  void (*async_string)(const char* str, uint64_t id, orbit_api_color color);
  void (*track_int)(const char* name, int value, orbit_api_color color);
  void (*track_int64)(const char* name, int64_t value, orbit_api_color color);
  void (*track_uint)(const char* name, uint32_t value, orbit_api_color color);
  void (*track_uint64)(const char* name, uint64_t value, orbit_api_color color);
  void (*track_float)(const char* name, float value, orbit_api_color color);
  void (*track_double)(const char* name, double value, orbit_api_color color);
};

struct orbit_api_v1 {
  uint32_t enabled;
  uint32_t initialized;
  void (*start)(const char* name, orbit_api_color color, uint64_t group_id,
                uint64_t caller_address);
  void (*stop)();
  void (*start_async)(const char* name, uint64_t id, orbit_api_color color);
  void (*stop_async)(uint64_t id);
  void (*async_string)(const char* str, uint64_t id, orbit_api_color color);
  void (*track_int)(const char* name, int value, orbit_api_color color);
  void (*track_int64)(const char* name, int64_t value, orbit_api_color color);
  void (*track_uint)(const char* name, uint32_t value, orbit_api_color color);
  void (*track_uint64)(const char* name, uint64_t value, orbit_api_color color);
  void (*track_float)(const char* name, float value, orbit_api_color color);
  void (*track_double)(const char* name, double value, orbit_api_color color);
};

#ifdef __linux

// And these are the versions that resulted from building Orbit.h on Windows, but defined on Linux
// in a way that yields the same result. They include previous versions, but also the current
// version, because we want the Windows layout to be available to us in liborbit.so when dealing
// with a Windows binary running on Wine.

struct orbit_api_win_v2 {
  uint32_t enabled;
  uint32_t initialized;
  __attribute__((ms_abi)) void (*start)(const char* name, orbit_api_color color, uint64_t group_id,
                                        uint64_t caller_address);
  __attribute__((ms_abi)) void (*stop)();
  __attribute__((ms_abi)) void (*start_async)(const char* name, uint64_t id, orbit_api_color color,
                                              uint64_t caller_address);
  __attribute__((ms_abi)) void (*stop_async)(uint64_t id);
  __attribute__((ms_abi)) void (*async_string)(const char* str, uint64_t id, orbit_api_color color);
  __attribute__((ms_abi)) void (*track_int)(const char* name, int value, orbit_api_color color);
  __attribute__((ms_abi)) void (*track_int64)(const char* name, int64_t value,
                                              orbit_api_color color);
  __attribute__((ms_abi)) void (*track_uint)(const char* name, uint32_t value,
                                             orbit_api_color color);
  __attribute__((ms_abi)) void (*track_uint64)(const char* name, uint64_t value,
                                               orbit_api_color color);
  __attribute__((ms_abi)) void (*track_float)(const char* name, float value, orbit_api_color color);
  __attribute__((ms_abi)) void (*track_double)(const char* name, double value,
                                               orbit_api_color color);
};

#endif  // __linux

#endif  // ORBIT_API_ORBIT_API_VERSIONS_H_
