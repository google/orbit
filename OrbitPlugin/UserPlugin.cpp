// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "UserPlugin.h"

#include "../OrbitCore/Platform.h"

//-----------------------------------------------------------------------------
void UserPlugin::Draw(ImGuiContext* a_ImguiContext, int a_Width, int a_Height) {
  ImGui::SetCurrentContext(a_ImguiContext);

  if (ImGui::Button("Plugin Test Button!!")) {
    OutputDebugString("Plugin button!\n");
  }
}

//-----------------------------------------------------------------------------
void UserPlugin::ReceiveUserData(const Orbit::UserData* a_Data) {}

//-----------------------------------------------------------------------------
void UserPlugin::ReceiveOrbitData(const Orbit::Data* a_Data) {}