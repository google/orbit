#include "SymbolsManager.h"

#include "Capture.h"
#include "ConnectionManager.h"
#include "CoreApp.h"
#include "OrbitBase/Logging.h"
#include "OrbitModule.h"
#include "SymbolHelper.h"
#include "TransactionManager.h"

namespace orbit {

SymbolsManager& SymbolsManager::Get() {
  static SymbolsManager symbols_manager;
  return symbols_manager;
}

void SymbolsManager::Init() {
  auto on_response = [this](const Message& msg) { HandleResponse(msg); };
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  TransactionManager::Get().RegisterTransactionHandler(
      {on_request, on_response, Msg_DebugSymbols, "Debug Symbols"});
}

void SymbolsManager::LoadSymbols(std::shared_ptr<Session> session,
                                 std::shared_ptr<Process> process) {
  session_ = session;
  std::vector<std::shared_ptr<Module>> modules;
  for (auto& pair : session->m_Modules) {
    const std::string& file_name = Path::GetFileName(pair.first);
    std::shared_ptr<Module> module = process->GetModuleFromName(file_name);
    if (module != nullptr) {
      modules.emplace_back(module);
    }
  }
  LoadSymbols(modules, process);
}

void SymbolsManager::LoadSymbols(
    const std::vector<std::shared_ptr<Module>>& modules,
    std::shared_ptr<Process> process) {
  if (modules.empty()) {
    ERROR("No module to load, cancelling.");
    return;
  }

  if (request_in_flight_) {
    ERROR("Module request already in flight, cancelling.");
    return;
  }

  request_in_flight_ = true;
  std::vector<ModuleDebugInfo> remote_module_infos;

  for (std::shared_ptr<Module> module : modules) {
    if (module == nullptr) continue;
    ModuleDebugInfo module_info;
    const char* name = module->m_Name.c_str();
    module_info.m_Name = name;
    module_info.m_PID = process->GetID();

    // Try to load modules from local machine.
    const SymbolHelper symbolHelper;
    if (symbolHelper.LoadSymbolsUsingSymbolsFile(module)) {
      symbolHelper.FillDebugInfoFromModule(module, module_info);
      module_infos_.emplace_back(std::move(module_info));
      size_t num_functions = module_info.m_Functions.size();
      LOG("Loaded %lu function symbols locally for %s", num_functions, name);
    } else {
      LOG("Did not find local symbols for module %s", name);
      remote_module_infos.push_back(std::move(module_info));
    }
  }

  // Don't request anything from service if we found all modules locally.
  if (remote_module_infos.empty()) {
    ProcessModuleInfos();
    return;
  }

  // Send request to service for modules that were not found locally.
  TransactionManager::EnqueueRequest(Msg_DebugSymbols, remote_module_infos);
}

void SymbolsManager::HandleRequest(const Message& message) {
  CHECK(ConnectionManager::Get().IsService());

  // Deserialize request message.
  std::vector<ModuleDebugInfo> module_infos;
  TransactionManager::ReceiveRequest(message, &module_infos);

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
    const SymbolHelper symbolHelper;
    if (symbolHelper.LoadSymbolsCollector(module)) {
      symbolHelper.FillDebugInfoFromModule(module, module_info);
      LOG("Loaded %lu function symbols for module %s",
          module_info.m_Functions.size(), module_name.c_str());
    } else {
      ERROR("Unable to load symbols of module %s", module->m_Name.c_str());
    }
  }

  // Send response to the client.
  TransactionManager::SendResponse(message.GetType(), module_infos);
}

void SymbolsManager::HandleResponse(const Message& message) {
  CHECK(ConnectionManager::Get().IsClient());

  // Deserialize response message.
  std::vector<ModuleDebugInfo> infos;
  TransactionManager::ReceiveResponse(message, &infos);

  // Append new debug information to our initial list and process them.
  module_infos_.insert(module_infos_.end(), infos.begin(), infos.end());
  ProcessModuleInfos();
}

void SymbolsManager::ProcessModuleInfos() {
  // Notify app of new debug symbols.
  GCoreApp->OnRemoteModuleDebugInfo(module_infos_);

  // Apply session.
  GCoreApp->ApplySession(session_);

  // Clear.
  module_infos_.clear();
  session_ = nullptr;
  request_in_flight_ = false;
}

}  // namespace orbit
