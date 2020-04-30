#include "OrbitService.h"

#include "ConnectionManager.h"
#include "Core.h"
#include "CoreApp.h"
#include "TcpServer.h"

OrbitService::OrbitService(uint16_t port) {
  // TODO: These should be private fields.
  GTcpServer = std::make_unique<TcpServer>();
  GTcpServer->StartServer(port);

  ConnectionManager::Get().InitAsService();

  core_app_ = std::make_unique<CoreApp>();
  GCoreApp = core_app_.get();

  transaction_manager_ = std::make_unique<orbit::TransactionManager>(
      GTcpClient.get(), GTcpServer.get());
  symbols_service_ = std::make_unique<SymbolsService>(
      &ConnectionManager::Get().GetProcessList(), transaction_manager_.get());
  SetupServiceMemoryTransaction();
}

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  while (!(*exit_requested)) {
    GTcpServer->ProcessMainThreadCallbacks();
    Sleep(16);
  }

  GCoreApp = nullptr;
}

void OrbitService::SetupServiceMemoryTransaction() {
  auto on_request = [this](const Message& msg) {
    // Receive request.
    std::tuple<uint32_t, uint64_t, uint64_t> pid_address_size;
    transaction_manager_->ReceiveRequest(msg, &pid_address_size);
    uint32_t pid = std::get<0>(pid_address_size);
    uint64_t address = std::get<1>(pid_address_size);
    uint64_t size = std::get<2>(pid_address_size);

    // Read target process memory.
    std::vector<uint8_t> bytes(size);
    uint64_t num_bytes_read = 0;
    if (!ReadProcessMemory(pid, address, bytes.data(), size, &num_bytes_read)) {
      ERROR("ReadProcessMemory error attempting to read %#lx", address);
    }
    bytes.resize(num_bytes_read);

    // Send response.
    transaction_manager_->SendResponse(msg.GetType(), bytes);
  };

  transaction_manager_->RegisterTransactionHandler(
      {on_request, nullptr, Msg_MemoryTransfer, "Memory Transfer"});
}
