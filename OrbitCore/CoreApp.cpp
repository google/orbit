//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#include "CoreApp.h"

#include "Capture.h"
#include "ConnectionManager.h"
#include "OrbitBase/Logging.h"
#include "TcpClient.h"

CoreApp* GCoreApp;

void CoreApp::InitializeClientTransactions() {
  CHECK(ConnectionManager::Get().IsClient());

  transaction_manager_ = std::make_unique<orbit::TransactionManager>(
      GTcpClient.get(), GTcpServer.get());
  symbols_client_ =
      std::make_unique<SymbolsClient>(GCoreApp, transaction_manager_.get());
  process_memory_client_ =
      std::make_unique<ProcessMemoryClient>(transaction_manager_.get());
}
