// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_ORBIT_V0_H_
#define ORBIT_API_ORBIT_V0_H_

#include <stdbool.h>
#include <stdint.h>

// =================================================================================================
// Orbit Manual Instrumentation API.
// =================================================================================================
#ifdef __linux
#define ORBIT_EXPORT __attribute__((visibility("default")))
#else
#define ORBIT_EXPORT __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}  // extern "C"

#endif  // __cplusplus

#endif  // ORBIT_API_ORBIT_V0_H_
