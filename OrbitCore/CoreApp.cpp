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
  SetupClientMemoryTransaction();
}

void CoreApp::GetRemoteMemory(
    uint32_t pid, uint64_t address, uint64_t size,
    std::function<void(std::vector<uint8_t>&)> callback) {
  absl::MutexLock lock(&transaction_mutex_);
  std::tuple<uint32_t, uint64_t, uint64_t> pid_address_size(pid, address, size);
  uint32_t id = transaction_manager_->EnqueueRequest(Msg_MemoryTransfer,
                                                     pid_address_size);
  memory_callbacks_[id] = callback;
}

void CoreApp::SetupClientMemoryTransaction() {
  auto on_response = [this](const Message& msg, uint32_t id) {
    std::vector<uint8_t> bytes;
    transaction_manager_->ReceiveResponse(msg, &bytes);
    absl::MutexLock lock(&transaction_mutex_);
    memory_callbacks_[id](bytes);
    memory_callbacks_.erase(id);
  };

  transaction_manager_->RegisterTransactionHandler(
      {nullptr, on_response, Msg_MemoryTransfer, "Memory Transfer"});
}
