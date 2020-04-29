#include "OrbitService.h"

#include "ConnectionManager.h"
#include "Core.h"
#include "CoreApp.h"
#include "TcpServer.h"

// TODO: we should probably make it configurable
constexpr uint32_t kAsioServerPort = 44766;

OrbitService::OrbitService() {
  // TODO: These should be private fields.
  GTcpServer = std::make_unique<TcpServer>();
  GTcpServer->StartServer(kAsioServerPort);

  ConnectionManager::Get().InitAsService();

  core_app_ = std::make_unique<CoreApp>();
  GCoreApp = core_app_.get();
  GCoreApp->InitializeManagers();
}

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  while (!(*exit_requested)) {
    GTcpServer->ProcessMainThreadCallbacks();
    Sleep(16);
  }

  GCoreApp = nullptr;
}
