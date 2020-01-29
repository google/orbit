#include "OrbitService.h"

#include <iostream>

#include "App.h"
#include "Core.h"

//-----------------------------------------------------------------------------
OrbitService::OrbitService() {
  OrbitApp::Init();
  GOrbitApp->SetHeadless(true);
  GOrbitApp->SetCommandLineArguments({"headless"});
  GOrbitApp->PostInit();
}

//-----------------------------------------------------------------------------
OrbitService::~OrbitService() {}

//-----------------------------------------------------------------------------
void OrbitService::Run() {
  while (!m_ExitRequested) {
    OrbitApp::MainTick();
    Sleep(16);
  }
}