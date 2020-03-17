#include "OrbitService.h"

#include <iostream>

#include "Capture.h"
#include "ConnectionManager.h"
#include "Core.h"
#include "TimerManager.h"
#include "TcpServer.h"

OrbitService::OrbitService() {
  // TODO: these should be a private fields.
  GTimerManager = std::make_unique<TimerManager>();
  GTcpServer = new TcpServer();
  Capture::Init();

  GTcpServer->Start(Capture::GCapturePort);
  ConnectionManager::Get().InitAsService();
}

void OrbitService::Run() {
  while (!exit_requested_) {
    GTcpServer->ProcessMainThreadCallbacks();
    Capture::Update();
    Sleep(16);
  }
}

