#pragma once
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "OrbitSdk.h"

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