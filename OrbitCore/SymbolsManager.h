#ifndef ORBIT_CORE_SYMBOLS_MANAGER_H_
#define ORBIT_CORE_SYMBOLS_MANAGER_H_

#include <atomic>
#include <memory>

#include "Message.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "OrbitSession.h"
#include "TransactionManager.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

class CoreApp;

namespace orbit {

// The role of the SymbolsManager is to load debug symbol information either
// from the developer's machine, or remotely on the target instance if symbols
// weren't found locally.

class SymbolsManager {
 public:
  SymbolsManager(CoreApp* core_app);

  void LoadSymbols(std::shared_ptr<Session> session,
                   std::shared_ptr<Process> process);
  void LoadSymbols(const std::vector<std::shared_ptr<Module>>& modules,
                   std::shared_ptr<Process> process,
                   std::shared_ptr<Session> session = nullptr);

  SymbolsManager() = delete;
  SymbolsManager(const SymbolsManager&) = delete;
  SymbolsManager& operator=(const SymbolsManager&) = delete;
  SymbolsManager(SymbolsManager&&) = delete;
  SymbolsManager& operator=(SymbolsManager&&) = delete;

private:
  void HandleRequest(const Message& message);
  void HandleResponse(const Message& message, uint32_t id);
  void FinalizeTransaction(Session* session);

  CoreApp* core_app_ = nullptr;
  TransactionManager* transaction_manager_;
  absl::flat_hash_map<uint32_t, std::shared_ptr<Session>> id_sessions_;
  absl::Mutex mutex_;
};

}  // namespace orbit

#endif  // ORBIT_CORE_SESSION_MANAGER_H_
