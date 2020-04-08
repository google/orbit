//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#include "CoreApp.h"
#include "TcpClient.h"

CoreApp* GCoreApp;

void CoreApp::InitializeManagers() {
  transaction_manager_ = std::make_unique<orbit::TransactionManager>(
    GTcpClient.get(), GTcpServer.get());
  symbols_manager_ = std::make_unique<orbit::SymbolsManager>(GCoreApp);
}
