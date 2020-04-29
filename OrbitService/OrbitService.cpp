#include "OrbitService.h"

#include <iostream>

#include "Capture.h"
#include "ConnectionManager.h"
#include "Core.h"
#include "CoreApp.h"
#include "TcpServer.h"
#include "TimerManager.h"

// TODO: we should probably make it configurable
constexpr uint32_t kAsioServerPort = 44766;

OrbitService::OrbitService() {
  // TODO: these should be a private fields.
  GTimerManager = std::make_unique<TimerManager>();
  GTcpServer = std::make_unique<TcpServer>();
  Capture::Init();

  GTcpServer->StartServer(kAsioServerPort);
  ConnectionManager::Get().InitAsService();

  core_app_ = std::make_unique<CoreApp>();
  GCoreApp = core_app_.get();
  GCoreApp->InitializeManagers();
}

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  while (!(*exit_requested)) {
    GTcpServer->ProcessMainThreadCallbacks();
    Capture::Update();
    Sleep(16);
  }

  GCoreApp = nullptr;
}
