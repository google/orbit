// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "OrbitSdk.h"
#include "imgui.h"
#include "imgui_internal.h"

//-----------------------------------------------------------------------------
class UserPlugin : public Orbit::Plugin {
 public:
  UserPlugin() {}
  virtual ~UserPlugin() {}

  virtual void Update() {}
  virtual const char* GetName() { return "UserPlugin"; }
  virtual void Draw(ImGuiContext* a_ImguiContext, int a_Width, int a_Height);
  virtual void ReceiveUserData(const Orbit::UserData* a_Data);
  virtual void ReceiveOrbitData(const Orbit::Data* a_Data);
};

extern "C" {
__declspec(dllexport) void* __cdecl CreateOrbitPlugin() {
  return new UserPlugin();
}
}
