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
    uint32_t pid, uint64_t address, uint64_t size,
    std::function<void(std::vector<byte>&)> callback) {
  absl::MutexLock lock(&transaction_mutex_);
  std::tuple<uint32_t, uint64_t, uint64_t> pid_address_size(pid, address, size);
  uint32_t id = transaction_manager_->EnqueueRequest(Msg_MemoryTransfer,
                                                     pid_address_size);
  memory_callbacks_[id] = callback;
}

void CoreApp::SetupMemoryTransaction() {
  auto on_request = [this](const Message& msg) {
    // Receive request.
    CHECK(IsService());
    std::tuple<uint32_t, uint64_t, uint64_t> pid_address_size;
    transaction_manager_->ReceiveRequest(msg, &pid_address_size);
    uint32_t pid = std::get<0>(pid_address_size);
    uint64_t address = std::get<1>(pid_address_size);
    uint64_t size = std::get<2>(pid_address_size);

    // read target process memory
    std::vector<byte> bytes(size);
    uint64_t num_bytes_read = 0;
    if (!ReadProcessMemory(pid, address, bytes.data(), size, &num_bytes_read)) {
      ERROR("ReadProcessMemory error attempting to read 0x%lx", address);
    }
    bytes.resize(num_bytes_read);

    // Send response.
    transaction_manager_->SendResponse(msg.GetType(), bytes);
  };

  auto on_response = [this](const Message& msg, uint32_t id) {
    CHECK(IsClient());
    std::vector<byte> bytes;
    transaction_manager_->ReceiveResponse(msg, &bytes);
    absl::MutexLock lock(&transaction_mutex_);
    memory_callbacks_[id](bytes);
    memory_callbacks_.erase(id);
  };

  transaction_manager_->RegisterTransactionHandler(
      {on_request, on_response, Msg_MemoryTransfer, "Memory Transfer"});
}
