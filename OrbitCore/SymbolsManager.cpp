#include "SymbolsManager.h"

#include "Capture.h"
#include "ConnectionManager.h"
#include "CoreApp.h"
#include "OrbitBase/Logging.h"
#include "OrbitModule.h"
#include "SymbolHelper.h"
#include "TransactionManager.h"

namespace orbit {

SymbolsManager::SymbolsManager(CoreApp* core_app) {
  core_app_ = core_app;
  transaction_manager_ = core_app_->GetTransactionManager();
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  auto on_response = [this](const Message& msg, uint32_t id) {
    HandleResponse(msg, id);
  };
  transaction_manager_->RegisterTransactionHandler(
      {on_request, on_response, Msg_DebugSymbols, "Debug Symbols"});
}

void SymbolsManager::LoadSymbols(std::shared_ptr<Session> session,
                                 std::shared_ptr<Process> process) {
  std::vector<std::shared_ptr<Module>> modules;
  for (auto& pair : session->m_Modules) {
    const std::string& file_name = Path::GetFileName(pair.first);
    std::shared_ptr<Module> module = process->GetModuleFromName(file_name);
    if (module != nullptr) {
      modules.emplace_back(module);
    }
  }
  LoadSymbols(modules, process, session);
}

void SymbolsManager::LoadSymbols(
    const std::vector<std::shared_ptr<Module>>& modules,
    std::shared_ptr<Process> process, std::shared_ptr<Session> session) {
  if (modules.empty()) {
    ERROR("No module to load, cancelling.");
    return;
  }

  std::vector<ModuleDebugInfo> remote_module_infos;

  for (std::shared_ptr<Module> module : modules) {
    if (module == nullptr) continue;
    ModuleDebugInfo module_info;
    const char* name = module->m_Name.c_str();
    module_info.m_Name = name;
    module_info.m_PID = process->GetID();

    // Try to load modules from local machine.
    const SymbolHelper symbol_helper;
    if (symbol_helper.LoadSymbolsUsingSymbolsFile(module)) {
      symbol_helper.FillDebugInfoFromModule(module, module_info);
      size_t num_functions = module_info.m_Functions.size();
      LOG("Loaded %lu function symbols locally for %s", num_functions, name);
    } else {
      LOG("Did not find local symbols for module %s", name);
      remote_module_infos.push_back(std::move(module_info));
    }
  }

  // Don't request anything from service if we found all modules locally.
  if (remote_module_infos.empty()) {
    FinalizeTransaction(session.get());
    return;
  }

  // Send request to service for modules that were not found locally.
  uint32_t id = transaction_manager_->EnqueueRequest(Msg_DebugSymbols,
                                                     remote_module_infos);

  absl::MutexLock lock(&mutex_);
  id_sessions_[id] = session;
}

void SymbolsManager::HandleRequest(const Message& message) {
  CHECK(ConnectionManager::Get().IsService());

  // Deserialize request message.
  std::vector<ModuleDebugInfo> module_infos;
  transaction_manager_->ReceiveRequest(message, &module_infos);

  for (auto& module_info : module_infos) {
    // Find process.
    uint32_t pid = module_info.m_PID;
    std::shared_ptr<Process> process = nullptr;
    process = ConnectionManager::Get().GetProcessList().GetProcess(pid);
    if (process == nullptr) {
      ERROR("Unable to find process %u", pid);
      continue;
    }

    // Find module.
    const std::string& module_name = module_info.m_Name;
    std::shared_ptr<Module> module = process->GetModuleFromName(module_name);
    if (!module) {
      ERROR("Unable to find module %s", module_name.c_str());
      continue;
    }

    // Load debug information.
    const SymbolHelper symbol_helper;
    if (symbol_helper.LoadSymbolsCollector(module)) {
      symbol_helper.FillDebugInfoFromModule(module, module_info);
      LOG("Loaded %lu function symbols for module %s",
          module_info.m_Functions.size(), module_name.c_str());
    } else {
      ERROR("Unable to load symbols of module %s", module->m_Name.c_str());
    }
  }

  // Send response to the client.
  transaction_manager_->SendResponse(message.GetType(), module_infos);
}

void SymbolsManager::HandleResponse(const Message& message, uint32_t id) {
  CHECK(ConnectionManager::Get().IsClient());

  // Deserialize response message.
  std::vector<ModuleDebugInfo> infos;
  transaction_manager_->ReceiveResponse(message, &infos);

  // Notify app of new debug symbols.
  core_app_->OnRemoteModuleDebugInfo(infos);

  // Finalize transaction.
  mutex_.Lock();
  std::shared_ptr<Session> session = id_sessions_[id];
  id_sessions_.erase(id);
  mutex_.Unlock();
  FinalizeTransaction(session.get());
}

void SymbolsManager::FinalizeTransaction(Session* session) {
  // Apply session.
  if (session != nullptr) {
    core_app_->ApplySession(*session);
  }
}

}  // namespace orbit
