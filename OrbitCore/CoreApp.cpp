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
  transaction_client_ = std::make_unique<TransactionClient>(GTcpClient.get());
  symbols_client_ =
      std::make_unique<SymbolsClient>(GCoreApp, transaction_client_.get());
  process_memory_client_ =
      std::make_unique<ProcessMemoryClient>(transaction_client_.get());
}
