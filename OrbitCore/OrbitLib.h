/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#define ORBIT_FUNCTION
//#define ORBIT_FUNCTION UserScopeTimer functionTimer( __FUNCTION__ );

#include <string>

struct UserScopeTimer {
  UserScopeTimer(const char* a_Name);
  ~UserScopeTimer();
  bool m_Valid;
  enum { Datasize = 512 };
  char m_Data[Datasize];
};

namespace Orbit {
void Init(const std::string& a_Host);
void InitRemote(const std::string& a_Host);
void DeInit();
void Start();
void Stop();
}  // namespace Orbit
