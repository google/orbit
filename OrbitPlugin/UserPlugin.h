#pragma once
#include "OrbitCore/OrbitSDK.h"
#include "imgui.h"

#ifdef _WIN32
#define ORBIT_EXPORT __declspec(dllexport)
#else
#define ORBIT_EXPORT
#endif

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
ORBIT_EXPORT void* CreateOrbitPlugin() { return new UserPlugin(); }
}

#undef ORBIT_EXPORT