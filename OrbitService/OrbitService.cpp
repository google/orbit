#include "OrbitService.h"

#include <iostream>

#include "App.h"
#include "Core.h"
#include "DataView.h"

//-----------------------------------------------------------------------------
OrbitService::OrbitService() {
  OrbitApp::Init();
  GOrbitApp->SetHeadless(true);
  GOrbitApp->SetCommandLineArguments({"headless"});
  GOrbitApp->PostInit();

  DataView::Create(DataViewType::PROCESSES);
  DataView::Create(DataViewType::MODULES);
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
