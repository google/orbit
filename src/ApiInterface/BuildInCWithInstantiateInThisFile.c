// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Orbit.h"

ORBIT_API_INSTANTIATE

void BuildInCWithInstantiateInThisFile(void) {
  ORBIT_START("name");
  ORBIT_START_WITH_COLOR("name", (orbit_api_color)0xc01);
  ORBIT_START_WITH_GROUP_ID("name", 42);
  ORBIT_START_WITH_COLOR_AND_GROUP_ID("name", (orbit_api_color)0xc01, 42);

  ORBIT_STOP();

  ORBIT_START_ASYNC("name", 42);
  ORBIT_START_ASYNC_WITH_COLOR("name", 42, (orbit_api_color)0xc01);

  ORBIT_STOP_ASYNC(42);

  ORBIT_ASYNC_STRING("string", 42);
  ORBIT_ASYNC_STRING_WITH_COLOR("string", 42, (orbit_api_color)0xc01);

  ORBIT_INT("name", -42);
  ORBIT_INT_WITH_COLOR("name", -42, (orbit_api_color)0xc01);

  ORBIT_INT64("name", -42LL);
  ORBIT_INT64_WITH_COLOR("name", -42LL, (orbit_api_color)0xc01);

  ORBIT_UINT("name", 42U);
  ORBIT_UINT_WITH_COLOR("name", 42U, (orbit_api_color)0xc01);

  ORBIT_UINT64("name", 42ULL);
  ORBIT_UINT64_WITH_COLOR("name", 42ULL, (orbit_api_color)0xc01);

  ORBIT_FLOAT("name", 42.0f);
  ORBIT_FLOAT_WITH_COLOR("name", 42.0f, (orbit_api_color)0xc01);

  ORBIT_DOUBLE("name", 42.0);
  ORBIT_DOUBLE_WITH_COLOR("name", 42.0, (orbit_api_color)0xc01);
}
