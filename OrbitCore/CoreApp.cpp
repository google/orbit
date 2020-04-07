//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#include "CoreApp.h"
#include "TcpClient.h"

std::shared_ptr<CoreApp> GCoreApp;

void CoreApp::InitializeManagers()
{
  transaction_manager_ =
      std::make_shared<orbit::TransactionManager>(GTcpClient, GTcpServer);
  symbols_manager_ = std::make_shared<orbit::SymbolsManager>(transaction_manager_);
}
