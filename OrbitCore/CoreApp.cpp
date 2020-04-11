//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#include "CoreApp.h"

#include "Capture.h"
#include "ConnectionManager.h"
#include "OrbitBase/Logging.h"
#include "TcpClient.h"

CoreApp* GCoreApp;

void CoreApp::InitializeManagers() {
  transaction_manager_ = std::make_unique<orbit::TransactionManager>(
      GTcpClient.get(), GTcpServer.get());
  symbols_manager_ = std::make_unique<orbit::SymbolsManager>(GCoreApp);

  SetupMemoryTransaction();

  is_client_ = ConnectionManager::Get().IsClient();
  is_service_ = ConnectionManager::Get().IsService();
}

void CoreApp::GetRemoteMemory(
    uint64_t address, uint64_t size,
    std::function<void(std::vector<byte>&)> callback) {
  absl::MutexLock lock(&transaction_mutex_);
  if (memory_callbacks_.find(address) != memory_callbacks_.end()) {
    ERROR("Memory request already in flight for address 0x%lx", address);
    return;
  }
  memory_callbacks_[address] = callback;
  std::pair<uint64_t, uint64_t> address_size(address, size);
  transaction_manager_->EnqueueRequest(Msg_MemoryTransfer, address_size);
}

void CoreApp::SetupMemoryTransaction() {
  auto on_request = [this](const Message& msg) {
    // Receive request.
    CHECK(IsService());
    std::pair<uint64_t, uint64_t> address_size;
    transaction_manager_->ReceiveRequest(msg, &address_size);
    uint64_t address = address_size.first;
    uint64_t size = address_size.second;
    std::pair<uint64_t, std::vector<byte>> address_bytes(address, {});
    std::vector<byte>& bytes = address_bytes.second;
    bytes.resize(size);

    // read target process memory
    uint64_t num_bytes_read = 0;
    uint32_t pid = Capture::GTargetProcess->GetID();  // TODO: remove globals.
    if (ReadProcessMemory(pid, address, bytes.data(), size, &num_bytes_read) ==
        false) {
      ERROR("ReadProcessMemory error attempting to read 0x%lx", address);
    }
    bytes.resize(num_bytes_read);

    // Send response.
    transaction_manager_->SendResponse(msg.GetType(), address_bytes);
  };

  auto on_response = [this](const Message& msg) {
    CHECK(IsClient());
    std::pair<uint64_t, std::vector<byte>> address_bytes;
    transaction_manager_->ReceiveResponse(msg, &address_bytes);
    uint64_t address = address_bytes.first;
    absl::MutexLock lock(&transaction_mutex_);
    memory_callbacks_[address](address_bytes.second);
    memory_callbacks_.erase(address);
  };

  transaction_manager_->RegisterTransactionHandler(
      {on_request, on_response, Msg_MemoryTransfer, "Memory Transfer"});
}
