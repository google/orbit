#ifndef ORBIT_CORE_SYMBOLS_MANAGER_H_
#define ORBIT_CORE_SYMBOLS_MANAGER_H_

#include <atomic>
#include <memory>

#include "Message.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "OrbitSession.h"

namespace orbit {

// The role of the SymbolsManager is to load debug symbol information either
// from the developer's machine, or remotely on the target instance if symbols
// weren't found locally.

class SymbolsManager {
 public:
  static SymbolsManager& Get();
  void Init();

  void LoadSymbols(std::shared_ptr<Session> session,
                   std::shared_ptr<Process> process);
  void LoadSymbols(const std::vector<std::shared_ptr<Module>>& modules,
                   std::shared_ptr<Process> process);

 private:
  SymbolsManager() = default;
  SymbolsManager(const SymbolsManager&) = delete;
  SymbolsManager& operator=(const SymbolsManager&) = delete;
  SymbolsManager(SymbolsManager&&) = delete;
  SymbolsManager& operator=(SymbolsManager&&) = delete;

  void HandleRequest(const Message& message);
  void HandleResponse(const Message& message);
  void ProcessModuleInfos();

  std::vector<ModuleDebugInfo> module_infos_;
  std::shared_ptr<Session> session_ = nullptr;
  std::atomic<bool> request_in_flight_ = false;
};

}  // namespace orbit

#endif  // ORBIT_CORE_SESSION_MANAGER_H_
